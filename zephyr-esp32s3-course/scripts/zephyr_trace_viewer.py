#!/usr/bin/env python3

"""
Zephyr CTF Trace Viewer (Tkinter)

Features:
- Load CTF directory via bt2
- Optional metadata JSON mapping
- Timeline canvas with zoom/pan/select
- Synchronized event table and details
- Filtering by time/event/lane/text/cpu/channel
- Optional interval bars from enter/exit pairing rules
- Export filtered events to CSV/JSON
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import re
import tkinter as tk
from dataclasses import asdict, dataclass
from pathlib import Path
from tkinter import filedialog, messagebox, ttk
from typing import Any, Iterable

try:
    import bt2  # type: ignore
except ImportError:  # pragma: no cover - runtime dependency
    bt2 = None


@dataclass
class EventRecord:
    eid: int
    ts_ns: int
    t_rel_us: float
    event_name: str
    cpu: int | None
    channel: str
    thread_id: int | None
    thread_name: str | None
    obj_id: int | None
    arg0: int | None
    arg1: int | None
    ret: int | None
    timeout: int | None
    payload_str: str
    context_str: str
    lane: str
    fields: dict[str, Any]


@dataclass
class IntervalRecord:
    start_eid: int
    end_eid: int
    start_us: float
    end_us: float
    dur_us: float
    lane: str
    kind: str
    key: str


def _to_builtin(value: Any) -> Any:
    if value is None:
        return None
    if isinstance(value, (str, int, float, bool)):
        return value
    try:
        return int(value)
    except Exception:
        pass
    try:
        return float(value)
    except Exception:
        pass
    if isinstance(value, (list, tuple)):
        return [_to_builtin(v) for v in value]
    if isinstance(value, dict):
        return {str(k): _to_builtin(v) for k, v in value.items()}
    # bt2 struct-like objects often support keys()/getitem
    try:
        keys = list(value.keys())  # type: ignore[attr-defined]
        return {str(k): _to_builtin(value[k]) for k in keys}
    except Exception:
        pass
    try:
        return [_to_builtin(v) for v in value]
    except Exception:
        return str(value)


def _flatten(prefix: str, value: Any, out: dict[str, Any]) -> None:
    if isinstance(value, dict):
        for k, v in value.items():
            key = f"{prefix}.{k}" if prefix else str(k)
            _flatten(key, v, out)
    else:
        out[prefix] = value


def _pick_int(flat: dict[str, Any], exact: Iterable[str], contains: Iterable[str]) -> int | None:
    for key in exact:
        if key in flat:
            try:
                return int(flat[key])
            except Exception:
                pass
    for key, val in flat.items():
        low = key.lower()
        if any(token in low for token in contains):
            try:
                return int(val)
            except Exception:
                pass
    return None


def _pick_str(flat: dict[str, Any], exact: Iterable[str], contains: Iterable[str]) -> str | None:
    for key in exact:
        if key in flat and isinstance(flat[key], str) and flat[key]:
            return str(flat[key])
    for key, val in flat.items():
        low = key.lower()
        if any(token in low for token in contains) and isinstance(val, str) and val:
            return str(val)
    return None


def _clock_ns_from_msg(msg: Any) -> int | None:
    snap = getattr(msg, "default_clock_snapshot", None)
    if snap is None:
        return None
    for attr in ("ns_from_origin", "value"):
        try:
            v = getattr(snap, attr)
            if callable(v):
                v = v()
            return int(v)
        except Exception:
            continue
    try:
        return int(snap)
    except Exception:
        return None


def load_ctf_events(trace_dir: Path, lane_mode: str, thread_name_map: dict[int, str] | None = None) -> list[EventRecord]:
    if bt2 is None:
        raise RuntimeError("bt2 not available. Install Babeltrace 2 Python bindings.")
    if not trace_dir.exists() or not trace_dir.is_dir():
        raise RuntimeError(f"Trace directory not found: {trace_dir}")
    if not (trace_dir / "metadata").exists():
        raise RuntimeError(f"'metadata' file not found in: {trace_dir}")

    events: list[EventRecord] = []
    first_ns: int | None = None
    tid_to_name: dict[int, str] = dict(thread_name_map or {})

    msg_it = bt2.TraceCollectionMessageIterator(str(trace_dir))
    for msg in msg_it:
        if not hasattr(msg, "event"):
            continue
        event = msg.event
        ts_ns = _clock_ns_from_msg(msg)
        if ts_ns is None:
            # Keep ordering if timestamps are unavailable.
            ts_ns = len(events)
        if first_ns is None:
            first_ns = ts_ns
        t_rel_us = (ts_ns - first_ns) / 1000.0

        payload = _to_builtin(getattr(event, "payload_field", None))
        common_ctx = _to_builtin(getattr(event, "common_context_field", None))
        specific_ctx = _to_builtin(getattr(event, "specific_context_field", None))
        stream_ctx = None
        try:
            stream_ctx = _to_builtin(event.stream.packet.context_field)
        except Exception:
            stream_ctx = None

        flat: dict[str, Any] = {}
        for section_name, section in (
            ("payload", payload),
            ("common", common_ctx),
            ("specific", specific_ctx),
            ("stream", stream_ctx),
        ):
            if section is not None:
                _flatten(section_name, section, flat)

        event_name = str(getattr(event, "name", "unknown"))
        thread_id = _pick_int(
            flat,
            exact=("payload.thread_id", "common.thread_id", "payload.tid", "common.tid"),
            contains=("thread_id", ".tid", " tid", "thread"),
        )
        thread_name = _pick_str(
            flat,
            exact=("payload.thread_name", "common.thread_name", "payload.name", "common.name"),
            contains=("thread_name", ".name"),
        )
        if thread_name is None and thread_id is not None:
            thread_name = tid_to_name.get(thread_id)

        if thread_id is not None and thread_name and "unknown" not in thread_name.lower():
            tid_to_name[thread_id] = thread_name

        cpu = _pick_int(flat, exact=("common.cpu_id", "stream.cpu_id", "payload.cpu_id"), contains=("cpu",))
        channel_val: str
        try:
            stream_id = getattr(event.stream, "id", None)
            channel_val = str(stream_id) if stream_id is not None else "0"
        except Exception:
            channel_val = "0"

        obj_id = _pick_int(
            flat,
            exact=(
                "payload.mutex",
                "payload.sem",
                "payload.timer",
                "payload.obj_id",
                "common.obj_id",
            ),
            contains=("mutex", "semaphore", "sem", "timer", "obj"),
        )
        arg0 = _pick_int(flat, exact=("payload.arg0",), contains=(".arg0", " arg0"))
        arg1 = _pick_int(flat, exact=("payload.arg1",), contains=(".arg1", " arg1"))
        ret = _pick_int(flat, exact=("payload.ret", "payload.result"), contains=("ret", "result"))
        timeout = _pick_int(flat, exact=("payload.timeout",), contains=("timeout",))

        payload_str = json.dumps(payload, ensure_ascii=True, default=str) if payload is not None else ""
        context_join = {
            "common": common_ctx,
            "specific": specific_ctx,
            "stream": stream_ctx,
        }
        context_str = json.dumps(context_join, ensure_ascii=True, default=str)

        lane = resolve_lane(
            event_name=event_name,
            thread_name=thread_name,
            thread_id=thread_id,
            cpu=cpu,
            lane_mode=lane_mode,
        )

        events.append(
            EventRecord(
                eid=len(events),
                ts_ns=ts_ns,
                t_rel_us=t_rel_us,
                event_name=event_name,
                cpu=cpu,
                channel=channel_val,
                thread_id=thread_id,
                thread_name=thread_name,
                obj_id=obj_id,
                arg0=arg0,
                arg1=arg1,
                ret=ret,
                timeout=timeout,
                payload_str=payload_str,
                context_str=context_str,
                lane=lane,
                fields=flat,
            )
        )

    return events


def _to_opt_int(value: Any) -> int | None:
    if value is None:
        return None
    if isinstance(value, str) and not value.strip():
        return None
    try:
        return int(value)
    except Exception:
        return None


def _to_opt_float(value: Any, default: float = 0.0) -> float:
    try:
        return float(value)
    except Exception:
        return default


def load_events_from_json(path: Path, lane_mode: str) -> list[EventRecord]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, list):
        raise RuntimeError("JSON input must be a list of event objects.")

    out: list[EventRecord] = []
    for idx, item in enumerate(payload):
        if not isinstance(item, dict):
            continue
        event_name = str(item.get("event_name", "unknown"))
        thread_id = _to_opt_int(item.get("thread_id"))
        thread_name = item.get("thread_name")
        if thread_name is not None:
            thread_name = str(thread_name)
        cpu = _to_opt_int(item.get("cpu"))
        lane = resolve_lane(event_name, thread_name, thread_id, cpu, lane_mode)
        fields = item.get("fields", {})
        if not isinstance(fields, dict):
            fields = {}
        out.append(
            EventRecord(
                eid=_to_opt_int(item.get("eid")) or idx,
                ts_ns=_to_opt_int(item.get("ts_ns")) or idx,
                t_rel_us=_to_opt_float(item.get("t_rel_us"), float(idx)),
                event_name=event_name,
                cpu=cpu,
                channel=str(item.get("channel", "0")),
                thread_id=thread_id,
                thread_name=thread_name,
                obj_id=_to_opt_int(item.get("obj_id")),
                arg0=_to_opt_int(item.get("arg0")),
                arg1=_to_opt_int(item.get("arg1")),
                ret=_to_opt_int(item.get("ret")),
                timeout=_to_opt_int(item.get("timeout")),
                payload_str=str(item.get("payload_str", "")),
                context_str=str(item.get("context_str", "")),
                lane=lane,
                fields=fields,
            )
        )

    out.sort(key=lambda e: e.t_rel_us)
    for i, ev in enumerate(out):
        ev.eid = i
    return out


def load_events_from_csv(path: Path, lane_mode: str) -> list[EventRecord]:
    out: list[EventRecord] = []
    with open(path, "r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        for idx, row in enumerate(reader):
            event_name = str(row.get("event_name", "unknown"))
            thread_id = _to_opt_int(row.get("thread_id"))
            thread_name = row.get("thread_name")
            if thread_name is not None and not str(thread_name).strip():
                thread_name = None
            cpu = _to_opt_int(row.get("cpu"))
            lane = resolve_lane(event_name, str(thread_name) if thread_name else None, thread_id, cpu, lane_mode)
            out.append(
                EventRecord(
                    eid=idx,
                    ts_ns=_to_opt_int(row.get("ts_ns")) or idx,
                    t_rel_us=_to_opt_float(row.get("t_rel_us"), float(idx)),
                    event_name=event_name,
                    cpu=cpu,
                    channel=str(row.get("channel", "0")),
                    thread_id=thread_id,
                    thread_name=str(thread_name) if thread_name else None,
                    obj_id=_to_opt_int(row.get("obj_id")),
                    arg0=_to_opt_int(row.get("arg0")),
                    arg1=_to_opt_int(row.get("arg1")),
                    ret=_to_opt_int(row.get("ret")),
                    timeout=_to_opt_int(row.get("timeout")),
                    payload_str=str(row.get("payload_str", "")),
                    context_str=str(row.get("context_str", "")),
                    lane=lane,
                    fields={},
                )
            )
    out.sort(key=lambda e: e.t_rel_us)
    for i, ev in enumerate(out):
        ev.eid = i
    return out


def resolve_lane(
    event_name: str,
    thread_name: str | None,
    thread_id: int | None,
    cpu: int | None,
    lane_mode: str,
) -> str:
    if lane_mode == "cpu":
        return f"CPU{cpu}" if cpu is not None else "CPU?"
    if lane_mode == "category":
        return event_name.split("_", 1)[0] if "_" in event_name else event_name
    if thread_name:
        return thread_name
    if thread_id is not None:
        return f"tid:{thread_id}"
    return event_name


def default_pairing_rules() -> list[dict[str, Any]]:
    return [
        {
            "kind": "mutex_lock",
            "start": r".*mutex.*lock.*enter$",
            "end": r".*mutex.*lock.*exit$",
            "key": "obj_or_thread",
        },
        {
            "kind": "semaphore_take",
            "start": r".*sem.*take.*enter$|.*semaphore.*take.*enter$",
            "end": r".*sem.*take.*exit$|.*semaphore.*take.*exit$",
            "key": "obj_or_thread",
        },
        {
            "kind": "semaphore_blocking",
            "start": r".*sem.*take.*enter$|.*semaphore.*take.*enter$",
            "end": r".*sem.*take.*blocking$|.*semaphore.*take.*blocking$",
            "key": "obj_or_thread",
        },
        {
            "kind": "thread_running",
            "start": r".*thread.*switched_in$",
            "end": r".*thread.*switched_out$",
            "key": "thread",
        },
    ]


def build_intervals(events: list[EventRecord], rules: list[dict[str, Any]]) -> list[IntervalRecord]:
    pending: dict[tuple[str, str], list[EventRecord]] = {}
    out: list[IntervalRecord] = []

    compiled: list[tuple[str, re.Pattern[str], re.Pattern[str], str]] = []
    for r in rules:
        compiled.append((r["kind"], re.compile(r["start"]), re.compile(r["end"]), r.get("key", "obj_or_thread")))

    def mk_key(ev: EventRecord, mode: str) -> str:
        if mode == "thread":
            if ev.thread_id is not None:
                return f"tid:{ev.thread_id}"
            return f"lane:{ev.lane}"
        if ev.obj_id is not None:
            return f"obj:{ev.obj_id}"
        if ev.thread_id is not None:
            return f"tid:{ev.thread_id}"
        return f"lane:{ev.lane}"

    for ev in events:
        for kind, start_re, end_re, key_mode in compiled:
            k = mk_key(ev, key_mode)
            pkey = (kind, k)
            if start_re.match(ev.event_name):
                pending.setdefault(pkey, []).append(ev)
            elif end_re.match(ev.event_name):
                start_list = pending.get(pkey, [])
                if start_list:
                    s = start_list.pop(0)
                    dur = max(0.0, ev.t_rel_us - s.t_rel_us)
                    out.append(
                        IntervalRecord(
                            start_eid=s.eid,
                            end_eid=ev.eid,
                            start_us=s.t_rel_us,
                            end_us=ev.t_rel_us,
                            dur_us=dur,
                            lane=s.lane,
                            kind=kind,
                            key=k,
                        )
                    )
    return out


class TraceViewerApp(tk.Tk):
    MAX_TIMELINE_POINTS = 20000

    def __init__(
        self,
        trace_path: Path | None = None,
        meta_path: Path | None = None,
        json_path: Path | None = None,
        csv_path: Path | None = None,
    ) -> None:
        super().__init__()
        self.title("Zephyr CTF Trace Viewer")
        self.geometry("1480x900")

        self.events_all: list[EventRecord] = []
        self.events_filtered: list[EventRecord] = []
        self.intervals_filtered: list[IntervalRecord] = []
        self.meta: dict[str, Any] = {}

        self.lane_mode_var = tk.StringVar(value="thread")
        self.search_var = tk.StringVar(value="")
        self.time_start_var = tk.StringVar(value="")
        self.time_end_var = tk.StringVar(value="")
        self.event_regex_var = tk.BooleanVar(value=False)
        self.show_intervals_var = tk.BooleanVar(value=True)
        self.status_var = tk.StringVar(value="Load a trace directory to begin.")

        self.view_start_us = 0.0
        self.view_span_us = 1_000_000.0
        self.drag_x = None

        self.selected_eid: int | None = None
        self.lanes: list[str] = []
        self.tooltip: tk.Toplevel | None = None

        self._build_ui()
        self._bind_shortcuts()

        if meta_path:
            self.load_meta(meta_path, show_dialog=False)
        if trace_path:
            self.load_trace(trace_path)
        elif json_path:
            self.load_json(json_path)
        elif csv_path:
            self.load_csv(csv_path)

    def _build_ui(self) -> None:
        toolbar = ttk.Frame(self)
        toolbar.pack(fill=tk.X, padx=6, pady=4)

        ttk.Button(toolbar, text="Open Trace", command=self.open_trace_dialog).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Open JSON", command=self.open_json_dialog).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Open CSV", command=self.open_csv_dialog).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Open Metadata", command=self.open_meta_dialog).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="Export CSV", command=self.export_csv).pack(side=tk.LEFT, padx=10)
        ttk.Button(toolbar, text="Export JSON", command=self.export_json).pack(side=tk.LEFT, padx=2)

        ttk.Label(toolbar, text="Lane Mode").pack(side=tk.LEFT, padx=(20, 4))
        lane_mode = ttk.Combobox(
            toolbar, textvariable=self.lane_mode_var, values=["thread", "cpu", "category"], width=10, state="readonly"
        )
        lane_mode.pack(side=tk.LEFT)
        lane_mode.bind("<<ComboboxSelected>>", lambda _e: self.recompute_lane_mode())

        ttk.Checkbutton(toolbar, text="Show Intervals", variable=self.show_intervals_var, command=self.redraw_timeline).pack(
            side=tk.LEFT, padx=(20, 0)
        )

        ttk.Label(toolbar, textvariable=self.status_var).pack(side=tk.RIGHT)

        root_pane = ttk.Panedwindow(self, orient=tk.HORIZONTAL)
        root_pane.pack(fill=tk.BOTH, expand=True)

        filters = ttk.Frame(root_pane, width=340)
        root_pane.add(filters, weight=0)

        right = ttk.Panedwindow(root_pane, orient=tk.VERTICAL)
        root_pane.add(right, weight=1)

        timeline_container = ttk.Frame(right)
        right.add(timeline_container, weight=3)
        table_container = ttk.Frame(right)
        right.add(table_container, weight=2)

        self._build_filters(filters)
        self._build_timeline(timeline_container)
        self._build_table(table_container)

    def _build_filters(self, parent: ttk.Frame) -> None:
        row = 0
        ttk.Label(parent, text="Filters", font=("", 11, "bold")).grid(row=row, column=0, sticky="w", padx=8, pady=6)
        row += 1

        ttk.Label(parent, text="Time Start (us)").grid(row=row, column=0, sticky="w", padx=8)
        row += 1
        ttk.Entry(parent, textvariable=self.time_start_var).grid(row=row, column=0, sticky="ew", padx=8, pady=2)
        row += 1

        ttk.Label(parent, text="Time End (us)").grid(row=row, column=0, sticky="w", padx=8)
        row += 1
        ttk.Entry(parent, textvariable=self.time_end_var).grid(row=row, column=0, sticky="ew", padx=8, pady=2)
        row += 1

        ttk.Label(parent, text="Event Types").grid(row=row, column=0, sticky="w", padx=8)
        row += 1
        self.event_listbox = tk.Listbox(parent, selectmode=tk.EXTENDED, height=8, exportselection=False)
        self.event_listbox.grid(row=row, column=0, sticky="nsew", padx=8)
        row += 1
        ttk.Checkbutton(parent, text="Event filter uses regex", variable=self.event_regex_var).grid(
            row=row, column=0, sticky="w", padx=8
        )
        row += 1

        ttk.Label(parent, text="Lane Filter").grid(row=row, column=0, sticky="w", padx=8)
        row += 1
        self.lane_listbox = tk.Listbox(parent, selectmode=tk.EXTENDED, height=8, exportselection=False)
        self.lane_listbox.grid(row=row, column=0, sticky="nsew", padx=8)
        row += 1

        ttk.Label(parent, text="CPU Filter").grid(row=row, column=0, sticky="w", padx=8)
        row += 1
        self.cpu_listbox = tk.Listbox(parent, selectmode=tk.EXTENDED, height=4, exportselection=False)
        self.cpu_listbox.grid(row=row, column=0, sticky="nsew", padx=8)
        row += 1

        ttk.Label(parent, text="Channel Filter").grid(row=row, column=0, sticky="w", padx=8)
        row += 1
        self.channel_listbox = tk.Listbox(parent, selectmode=tk.EXTENDED, height=4, exportselection=False)
        self.channel_listbox.grid(row=row, column=0, sticky="nsew", padx=8)
        row += 1

        ttk.Label(parent, text="Text Search").grid(row=row, column=0, sticky="w", padx=8)
        row += 1
        ttk.Entry(parent, textvariable=self.search_var).grid(row=row, column=0, sticky="ew", padx=8, pady=2)
        row += 1

        btns = ttk.Frame(parent)
        btns.grid(row=row, column=0, sticky="ew", padx=8, pady=8)
        ttk.Button(btns, text="Apply", command=self.apply_filters).pack(side=tk.LEFT)
        ttk.Button(btns, text="Reset", command=self.reset_filters).pack(side=tk.LEFT, padx=6)

        parent.grid_columnconfigure(0, weight=1)
        parent.grid_rowconfigure(row - 3, weight=1)

    def _build_timeline(self, parent: ttk.Frame) -> None:
        self.timeline = tk.Canvas(parent, bg="#101218")
        self.timeline.pack(fill=tk.BOTH, expand=True)
        self.timeline.bind("<MouseWheel>", self.on_timeline_wheel)
        self.timeline.bind("<ButtonPress-1>", self.on_timeline_press)
        self.timeline.bind("<B1-Motion>", self.on_timeline_drag)
        self.timeline.bind("<ButtonRelease-1>", self.on_timeline_release)
        self.timeline.bind("<Motion>", self.on_timeline_motion)
        self.timeline.bind("<Leave>", self.hide_tooltip)
        self.timeline.bind("<Double-Button-1>", self.on_timeline_double_click)

    def _build_table(self, parent: ttk.Frame) -> None:
        cols = ("t_rel_us", "lane", "event_name", "cpu", "channel", "summary")
        self.table = ttk.Treeview(parent, columns=cols, show="headings", selectmode="browse")
        for c, w in (
            ("t_rel_us", 120),
            ("lane", 190),
            ("event_name", 280),
            ("cpu", 60),
            ("channel", 70),
            ("summary", 680),
        ):
            self.table.heading(c, text=c, command=lambda cc=c: self.sort_table(cc))
            self.table.column(c, width=w, stretch=True, anchor=tk.W)
        self.table.pack(fill=tk.BOTH, expand=True, side=tk.LEFT)
        self.table.bind("<<TreeviewSelect>>", self.on_table_select)

        vs = ttk.Scrollbar(parent, orient=tk.VERTICAL, command=self.table.yview)
        vs.pack(side=tk.LEFT, fill=tk.Y)
        self.table.configure(yscrollcommand=vs.set)

        details_frame = ttk.LabelFrame(parent, text="Details")
        details_frame.pack(fill=tk.X, side=tk.BOTTOM, padx=4, pady=4)
        self.details = tk.Text(details_frame, height=7, wrap="word")
        self.details.pack(fill=tk.BOTH, expand=True)

    def _bind_shortcuts(self) -> None:
        self.bind("<Control-o>", lambda _e: self.open_trace_dialog())
        self.bind("<Control-m>", lambda _e: self.open_meta_dialog())
        self.bind("<Control-f>", lambda _e: self.apply_filters())

    def open_trace_dialog(self) -> None:
        path = filedialog.askdirectory(title="Select CTF trace directory")
        if path:
            self.load_trace(Path(path))

    def open_json_dialog(self) -> None:
        path = filedialog.askopenfilename(
            title="Select events JSON",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
        )
        if path:
            self.load_json(Path(path))

    def open_csv_dialog(self) -> None:
        path = filedialog.askopenfilename(
            title="Select events CSV",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        )
        if path:
            self.load_csv(Path(path))

    def open_meta_dialog(self) -> None:
        path = filedialog.askopenfilename(
            title="Select metadata mapping JSON",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
        )
        if path:
            self.load_meta(Path(path), show_dialog=True)

    def load_meta(self, path: Path, show_dialog: bool) -> None:
        try:
            self.meta = json.loads(path.read_text(encoding="utf-8"))
            if show_dialog:
                messagebox.showinfo("Metadata", f"Loaded metadata:\n{path}")
            if self.events_all:
                self.recompute_lane_mode()
        except Exception as exc:
            messagebox.showerror("Metadata error", str(exc))

    def load_trace(self, trace_path: Path) -> None:
        if bt2 is None:
            messagebox.showerror(
                "Trace load error",
                "bt2 is not available in this Python environment.\n"
                "Use Open JSON/Open CSV in this app, or install bt2 and reopen.",
            )
            return
        self.status_var.set("Loading trace...")
        self.update_idletasks()
        try:
            thread_name_map = {}
            for k, v in self.meta.get("thread_names", {}).items():
                try:
                    thread_name_map[int(k)] = str(v)
                except Exception:
                    continue

            events = load_ctf_events(trace_path, lane_mode=self.lane_mode_var.get(), thread_name_map=thread_name_map)
            self._set_loaded_events(events, f"Loaded {len(events)} events from {trace_path}")
        except Exception as exc:
            messagebox.showerror("Trace load error", str(exc))
            self.status_var.set("Load failed.")

    def load_json(self, json_path: Path) -> None:
        self.status_var.set("Loading JSON...")
        self.update_idletasks()
        try:
            events = load_events_from_json(json_path, lane_mode=self.lane_mode_var.get())
            self._set_loaded_events(events, f"Loaded {len(events)} events from JSON {json_path}")
        except Exception as exc:
            messagebox.showerror("JSON load error", str(exc))
            self.status_var.set("Load failed.")

    def load_csv(self, csv_path: Path) -> None:
        self.status_var.set("Loading CSV...")
        self.update_idletasks()
        try:
            events = load_events_from_csv(csv_path, lane_mode=self.lane_mode_var.get())
            self._set_loaded_events(events, f"Loaded {len(events)} events from CSV {csv_path}")
        except Exception as exc:
            messagebox.showerror("CSV load error", str(exc))
            self.status_var.set("Load failed.")

    def _set_loaded_events(self, events: list[EventRecord], status_msg: str) -> None:
        if not events:
            raise RuntimeError("No events found.")
        self.events_all = events
        self._apply_object_and_thread_mappings()
        self.reset_filters(select_all=True)
        self.status_var.set(status_msg)

    def _apply_object_and_thread_mappings(self) -> None:
        obj_names = {str(k): str(v) for k, v in self.meta.get("object_names", {}).items()}
        thread_names = {str(k): str(v) for k, v in self.meta.get("thread_names", {}).items()}
        for ev in self.events_all:
            if ev.thread_id is not None:
                alias = thread_names.get(str(ev.thread_id))
                if alias:
                    ev.thread_name = alias
            if ev.obj_id is not None and str(ev.obj_id) in obj_names:
                ev.fields["meta.obj_name"] = obj_names[str(ev.obj_id)]
            ev.lane = resolve_lane(
                event_name=ev.event_name,
                thread_name=ev.thread_name,
                thread_id=ev.thread_id,
                cpu=ev.cpu,
                lane_mode=self.lane_mode_var.get(),
            )

    def recompute_lane_mode(self) -> None:
        if not self.events_all:
            return
        self._apply_object_and_thread_mappings()
        self.refresh_filter_lists()
        self.apply_filters()

    def refresh_filter_lists(self) -> None:
        event_types = sorted({ev.event_name for ev in self.events_all})
        lanes = sorted({ev.lane for ev in self.events_all})
        cpus = sorted({ev.cpu for ev in self.events_all if ev.cpu is not None})
        channels = sorted({ev.channel for ev in self.events_all})

        self._set_listbox_values(self.event_listbox, event_types)
        self._set_listbox_values(self.lane_listbox, lanes)
        self._set_listbox_values(self.cpu_listbox, [str(v) for v in cpus])
        self._set_listbox_values(self.channel_listbox, [str(v) for v in channels])

    @staticmethod
    def _set_listbox_values(listbox: tk.Listbox, values: list[str]) -> None:
        listbox.delete(0, tk.END)
        for v in values:
            listbox.insert(tk.END, v)

    @staticmethod
    def _selected_listbox_values(listbox: tk.Listbox) -> set[str]:
        selected = set()
        for idx in listbox.curselection():
            selected.add(str(listbox.get(idx)))
        return selected

    def reset_filters(self, select_all: bool = True) -> None:
        if not self.events_all:
            return
        min_us = self.events_all[0].t_rel_us
        max_us = self.events_all[-1].t_rel_us
        self.time_start_var.set(f"{min_us:.3f}")
        self.time_end_var.set(f"{max_us:.3f}")
        self.search_var.set("")
        self.refresh_filter_lists()

        if select_all:
            for lb in (self.event_listbox, self.lane_listbox, self.cpu_listbox, self.channel_listbox):
                lb.selection_set(0, tk.END)
        self.apply_filters()

    def apply_filters(self) -> None:
        if not self.events_all:
            return

        try:
            t0 = float(self.time_start_var.get().strip())
            t1 = float(self.time_end_var.get().strip())
        except ValueError:
            messagebox.showerror("Filter error", "Time range must be numeric.")
            return
        if t1 < t0:
            t0, t1 = t1, t0

        selected_events = self._selected_listbox_values(self.event_listbox)
        selected_lanes = self._selected_listbox_values(self.lane_listbox)
        selected_cpus = self._selected_listbox_values(self.cpu_listbox)
        selected_channels = self._selected_listbox_values(self.channel_listbox)
        search = self.search_var.get().strip().lower()

        event_patterns: list[re.Pattern[str]] = []
        if self.event_regex_var.get():
            for pat in selected_events:
                try:
                    event_patterns.append(re.compile(pat))
                except re.error as exc:
                    messagebox.showerror("Regex error", f"Invalid event regex '{pat}': {exc}")
                    return

        out: list[EventRecord] = []
        for ev in self.events_all:
            if ev.t_rel_us < t0 or ev.t_rel_us > t1:
                continue
            if selected_events:
                if self.event_regex_var.get():
                    if not any(p.search(ev.event_name) for p in event_patterns):
                        continue
                else:
                    if ev.event_name not in selected_events:
                        continue
            if selected_lanes and ev.lane not in selected_lanes:
                continue
            if selected_cpus and str(ev.cpu) not in selected_cpus:
                continue
            if selected_channels and str(ev.channel) not in selected_channels:
                continue
            if search:
                hay = f"{ev.payload_str} {ev.context_str} {ev.event_name} {ev.lane}".lower()
                if search not in hay:
                    continue
            out.append(ev)

        self.events_filtered = out
        self.intervals_filtered = self._build_intervals_for_filtered()
        self.lanes = self._ordered_lanes(self.events_filtered)

        if self.events_filtered:
            self.view_start_us = self.events_filtered[0].t_rel_us
            self.view_span_us = max(1000.0, self.events_filtered[-1].t_rel_us - self.events_filtered[0].t_rel_us)
        else:
            self.view_start_us = 0.0
            self.view_span_us = 1_000_000.0

        self.populate_table()
        self.redraw_timeline()
        self.status_var.set(f"Filtered: {len(self.events_filtered)} / {len(self.events_all)} events")

    def _build_intervals_for_filtered(self) -> list[IntervalRecord]:
        rules = self.meta.get("pairing_rules")
        if not rules:
            rules = default_pairing_rules()
        return build_intervals(self.events_filtered, rules)

    def _ordered_lanes(self, events: list[EventRecord]) -> list[str]:
        lanes = sorted({ev.lane for ev in events})
        lane_priority = self.meta.get("lane_priority", {})
        if lane_priority:
            lanes.sort(key=lambda l: (lane_priority.get(l, 10_000), l))
        return lanes

    def populate_table(self) -> None:
        self.table.delete(*self.table.get_children())
        for ev in self.events_filtered:
            summary = self._summary_for_event(ev)
            self.table.insert(
                "",
                tk.END,
                iid=str(ev.eid),
                values=(f"{ev.t_rel_us:.3f}", ev.lane, ev.event_name, ev.cpu, ev.channel, summary),
            )

    @staticmethod
    def _summary_for_event(ev: EventRecord) -> str:
        bits = []
        for key in ("arg0", "arg1", "ret", "timeout"):
            val = getattr(ev, key)
            if val is not None:
                bits.append(f"{key}={val}")
        if ev.obj_id is not None:
            bits.append(f"obj={ev.obj_id}")
        if "meta.obj_name" in ev.fields:
            bits.append(f"name={ev.fields['meta.obj_name']}")
        if not bits:
            payload = ev.payload_str
            bits.append(payload[:120] + ("..." if len(payload) > 120 else ""))
        return ", ".join(bits)

    def sort_table(self, column: str) -> None:
        if column == "t_rel_us":
            self.events_filtered.sort(key=lambda e: e.t_rel_us)
        elif column == "lane":
            self.events_filtered.sort(key=lambda e: (e.lane, e.t_rel_us))
        elif column == "event_name":
            self.events_filtered.sort(key=lambda e: (e.event_name, e.t_rel_us))
        else:
            return
        self.populate_table()
        self.redraw_timeline()

    def redraw_timeline(self) -> None:
        c = self.timeline
        c.delete("all")
        width = max(1, c.winfo_width())
        height = max(1, c.winfo_height())
        if not self.events_filtered:
            c.create_text(width // 2, height // 2, fill="#D0D2D8", text="No events in current filter")
            return

        lane_h = 32
        top_pad = 24
        left_pad = 170
        right_pad = 20
        visible_w = max(1, width - left_pad - right_pad)

        lane_to_y: dict[str, int] = {}
        for idx, lane in enumerate(self.lanes):
            y = top_pad + idx * lane_h
            lane_to_y[lane] = y
            c.create_text(8, y, anchor="w", fill="#AAB0BE", text=lane)
            c.create_line(left_pad, y, width - right_pad, y, fill="#202430")

        t0 = self.view_start_us
        t1 = self.view_start_us + self.view_span_us
        if t1 <= t0:
            t1 = t0 + 1.0

        def x_of(t_us: float) -> float:
            return left_pad + ((t_us - t0) / (t1 - t0)) * visible_w

        # Axis labels
        c.create_text(left_pad, 10, anchor="w", fill="#AAB0BE", text=f"{t0:.3f} us")
        c.create_text(width - right_pad, 10, anchor="e", fill="#AAB0BE", text=f"{t1:.3f} us")

        visible_events = [ev for ev in self.events_filtered if t0 <= ev.t_rel_us <= t1]
        step = max(1, math.ceil(len(visible_events) / self.MAX_TIMELINE_POINTS))

        if self.show_intervals_var.get():
            for iv in self.intervals_filtered:
                if iv.end_us < t0 or iv.start_us > t1:
                    continue
                y = lane_to_y.get(iv.lane)
                if y is None:
                    continue
                x0 = x_of(max(iv.start_us, t0))
                x1 = x_of(min(iv.end_us, t1))
                if x1 < x0:
                    x0, x1 = x1, x0
                c.create_rectangle(x0, y - 7, x1, y + 7, fill="#384A86", outline="")

        for i, ev in enumerate(visible_events):
            if i % step != 0:
                continue
            y = lane_to_y.get(ev.lane, top_pad)
            x = x_of(ev.t_rel_us)
            r = 3
            color = "#67D3E8"
            tags = ("event", f"eid:{ev.eid}")
            item = c.create_oval(x - r, y - r, x + r, y + r, fill=color, outline="", tags=tags)
            if ev.eid == self.selected_eid:
                c.itemconfigure(item, fill="#F4D35E")
                c.create_oval(x - 6, y - 6, x + 6, y + 6, outline="#F4D35E")

        c.config(scrollregion=(0, 0, width, top_pad + max(1, len(self.lanes)) * lane_h + 50))

    def on_timeline_wheel(self, event: tk.Event) -> None:
        if not self.events_filtered:
            return
        delta = -1 if event.delta < 0 else 1
        factor = 1.25 if delta < 0 else 0.8
        width = max(1, self.timeline.winfo_width() - 190)
        px = max(0, min(width, event.x - 170))
        frac = px / max(1, width)
        anchor_t = self.view_start_us + frac * self.view_span_us
        new_span = max(10.0, self.view_span_us * factor)
        self.view_start_us = anchor_t - frac * new_span
        self.view_span_us = new_span
        self.redraw_timeline()

    def on_timeline_press(self, event: tk.Event) -> None:
        self.drag_x = event.x
        self._select_event_near(event.x, event.y)

    def on_timeline_drag(self, event: tk.Event) -> None:
        if self.drag_x is None:
            return
        dx = event.x - self.drag_x
        self.drag_x = event.x
        width = max(1, self.timeline.winfo_width() - 190)
        dt = -(dx / max(1, width)) * self.view_span_us
        self.view_start_us += dt
        self.redraw_timeline()

    def on_timeline_release(self, _event: tk.Event) -> None:
        self.drag_x = None

    def on_timeline_double_click(self, event: tk.Event) -> None:
        self._select_event_near(event.x, event.y)
        if self.selected_eid is not None:
            self.center_on_eid(self.selected_eid)

    def _select_event_near(self, x: int, y: int) -> None:
        items = self.timeline.find_overlapping(x - 6, y - 6, x + 6, y + 6)
        for item in items:
            tags = self.timeline.gettags(item)
            for tag in tags:
                if tag.startswith("eid:"):
                    try:
                        eid = int(tag.split(":", 1)[1])
                        self.select_eid(eid)
                        return
                    except Exception:
                        continue

    def on_timeline_motion(self, event: tk.Event) -> None:
        items = self.timeline.find_overlapping(event.x - 4, event.y - 4, event.x + 4, event.y + 4)
        for item in items:
            for tag in self.timeline.gettags(item):
                if tag.startswith("eid:"):
                    try:
                        eid = int(tag.split(":", 1)[1])
                    except Exception:
                        continue
                    ev = self._event_by_eid(eid)
                    if ev:
                        self.show_tooltip(
                            event.x_root,
                            event.y_root,
                            f"eid={ev.eid}\n{ev.t_rel_us:.3f} us\n{ev.lane}\n{ev.event_name}",
                        )
                        return
        self.hide_tooltip()

    def show_tooltip(self, x_root: int, y_root: int, text: str) -> None:
        if self.tooltip is None:
            self.tooltip = tk.Toplevel(self)
            self.tooltip.wm_overrideredirect(True)
            lbl = ttk.Label(self.tooltip, text=text, background="#F5F7FA")
            lbl.pack(ipadx=6, ipady=4)
        else:
            lbl = self.tooltip.winfo_children()[0]
            lbl.configure(text=text)
        self.tooltip.geometry(f"+{x_root + 14}+{y_root + 14}")

    def hide_tooltip(self, _event: tk.Event | None = None) -> None:
        if self.tooltip is not None:
            self.tooltip.destroy()
            self.tooltip = None

    def on_table_select(self, _event: tk.Event) -> None:
        selected = self.table.selection()
        if not selected:
            return
        try:
            eid = int(selected[0])
        except Exception:
            return
        self.select_eid(eid, update_table=False)

    def select_eid(self, eid: int, update_table: bool = True) -> None:
        ev = self._event_by_eid(eid)
        if ev is None:
            return
        self.selected_eid = eid
        if update_table:
            self.table.selection_set(str(eid))
            self.table.see(str(eid))
        self.details.delete("1.0", tk.END)
        detail = {
            "eid": ev.eid,
            "t_rel_us": ev.t_rel_us,
            "event_name": ev.event_name,
            "lane": ev.lane,
            "thread_id": ev.thread_id,
            "thread_name": ev.thread_name,
            "cpu": ev.cpu,
            "channel": ev.channel,
            "obj_id": ev.obj_id,
            "payload_str": ev.payload_str,
            "context_str": ev.context_str,
            "fields": ev.fields,
        }
        self.details.insert("1.0", json.dumps(detail, indent=2, ensure_ascii=True, default=str))
        self.redraw_timeline()

    def _event_by_eid(self, eid: int) -> EventRecord | None:
        # eids are dense; direct scan keeps code simple for filtered subset.
        for ev in self.events_filtered:
            if ev.eid == eid:
                return ev
        return None

    def center_on_eid(self, eid: int) -> None:
        ev = self._event_by_eid(eid)
        if ev is None:
            return
        self.view_start_us = ev.t_rel_us - 0.5 * self.view_span_us
        self.redraw_timeline()

    def export_csv(self) -> None:
        if not self.events_filtered:
            messagebox.showinfo("Export", "No filtered events to export.")
            return
        path = filedialog.asksaveasfilename(
            defaultextension=".csv",
            filetypes=[("CSV", "*.csv"), ("All files", "*.*")],
            title="Export filtered events to CSV",
        )
        if not path:
            return
        cols = [
            "eid",
            "ts_ns",
            "t_rel_us",
            "event_name",
            "lane",
            "cpu",
            "channel",
            "thread_id",
            "thread_name",
            "obj_id",
            "arg0",
            "arg1",
            "ret",
            "timeout",
            "payload_str",
            "context_str",
        ]
        with open(path, "w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=cols)
            w.writeheader()
            for ev in self.events_filtered:
                row = {k: getattr(ev, k) for k in cols}
                w.writerow(row)
        self.status_var.set(f"Exported CSV: {path}")

    def export_json(self) -> None:
        if not self.events_filtered:
            messagebox.showinfo("Export", "No filtered events to export.")
            return
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON", "*.json"), ("All files", "*.*")],
            title="Export filtered events to JSON",
        )
        if not path:
            return
        data = [asdict(ev) for ev in self.events_filtered]
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=True, default=str)
        self.status_var.set(f"Exported JSON: {path}")


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Zephyr CTF Trace Viewer")
    p.add_argument("--trace", type=Path, default=None, help="Path to CTF trace directory")
    p.add_argument("--json", dest="events_json", type=Path, default=None, help="Path to pre-exported events JSON")
    p.add_argument("--csv", dest="events_csv", type=Path, default=None, help="Path to pre-exported events CSV")
    p.add_argument("--meta", type=Path, default=None, help="Path to optional metadata mapping JSON")
    return p.parse_args()


def main() -> None:
    args = parse_args()
    app = TraceViewerApp(
        trace_path=args.trace,
        meta_path=args.meta,
        json_path=args.events_json,
        csv_path=args.events_csv,
    )
    app.mainloop()


if __name__ == "__main__":
    main()
