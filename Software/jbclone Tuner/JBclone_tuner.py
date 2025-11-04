import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
from matplotlib import pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
from matplotlib.axes import Axes
import time
import serial
from serial.tools.list_ports import comports
from typing import Optional, List, Dict, Callable


class Station:
    """Represents a controller interface for a soldering station."""

    tips = {
        "T245": 0,
        "AM120-1": 1,
        "AM120-2": 2,
        "C360": 3,
    }

    class commands:
        """Command identifiers understood by the station."""

        enable = "en"
        temp_runaway = "runaway_t"
        temp_set_min = "set_min_t"
        temp_set_max = "set_max_t"
        temp_measure = "meas_t"
        temp_set = "set_t"
        tc_voltage_measure = "meas_uv"
        tc_voltage_setpoint = "set_uv"
        cal_tc_table = "tc_cal_table"
        pid_kp = "pid_kp"
        pid_ki = "pid_ki"
        pid_kd = "pid_kd"
        pid_derivative_filter_time = "pid_d_tau"
        pid_op = "pid_op"
        sleep_temp = "sleep_set_t"
        sleep_delay = "sleep_delay"
        sleep_state = "sleep_state"
        restore_default_config = "restore"

    def __init__(self):
        """Initialize station controller."""
        self.baud = 152000
        self.port: Optional[serial.Serial] = None
        self.connected = False
        self.tip_index = None
        self.tip_changed = False

    def connect(self, target_port: str) -> bool:
        """Connect to the station on the specified port."""
        try:
            if self.port is not None:
                self.disconnect()

            if target_port not in [i.name for i in list(comports())]:
                raise ValueError(f"Port {target_port} not available")

            self.port = serial.Serial(target_port, self.baud, timeout=1)

            if not self.port.is_open:
                self.disconnect()

            self.connected = True

        except Exception as e:
            self.disconnect()
            messagebox.showerror("Connection Error", str(e))

        return self.connected

    def disconnect(self) -> None:
        """Disconnect from the station and close the port."""
        if self.port is not None:
            self.port.close()
            self.port = None
            self.connected = False

    def validate_comand(self, command: str) -> None:
        """Validate a command string against known commands. raises an exception if invalid."""
        if self.port is None:
            raise Exception("Not connected to any station")

        valid_commands = [
            v for k, v in vars(self.commands).items() if not k.startswith("__")
        ]
        if command not in valid_commands:
            raise Exception(f"Invalid command: {command}")

    def set_target_tip(self, new_tip: str | int) -> bool:
        """Set the target tip index based on the provided string or integer.
        spoawns error dialog and returns false if the tip is invalid, True if good"""
        try:
            if isinstance(new_tip, int):
                self.tip_index = int(new_tip)
                if self.tip_index not in self.tips.values():
                    raise KeyError(f"Invalid tip index: {new_tip}")
            elif isinstance(new_tip, str):
                self.tip_index = self.tips[new_tip]
            else:
                raise ValueError(
                    f"Invalid tip type: {type(new_tip)}. Expected str or int."
                )
        except ValueError as e:
            messagebox.showerror("Error", str(e))
            return False
        except KeyError as e:
            messagebox.showerror("Error", str(e))
            return False
        return True

    def set(self, command: str, value=None) -> None:
        """Send a value-setting command to the station.
        shows messagebox if the response indicates failure.
        """
        try:
            self.validate_comand(command)

            command_str = f"{self.tip_index}:{command}:{value}\n"
            self.port.write(command_str.encode("ASCII"))
            response = self.port.read_until().decode().strip()

            error_preamble = "ERROR "
            if response.startswith(error_preamble):
                raise Exception(f"{command}:{response[len(error_preamble):]}")

            if response != "OK":
                raise Exception(f"Unexpected response: {response}")
        except Exception as e:
            messagebox.showerror("Unable To Set", str(e))

    def get(self, command: str, appendix=None) -> str | None:
        """Query a value from the station and return the response.
        show messagebox if response indicates failure.
        """
        try:
            self.validate_comand(command)
            arg = appendix if appendix is not None else "?"
            message = f"{self.tip_index}:{command}:{arg}"
            self.port.write(message.encode("ASCII"))
            response = self.port.read_until()
            response = response.decode().strip()

            error_preamble = "ERROR "
            if response.startswith(error_preamble):
                raise ValueError(f"{command}:{response[len(error_preamble):]}")
            return response

        except Exception as e:
            messagebox.showerror("Unable To Get", str(e))
            return None


def check_float_entry(event: tk.Event) -> bool:
    "check if the entry is a valid float and change color accordingly"
    try:
        float(event.widget.get())
        event.widget.config(bg="white")
        return True
    except ValueError:
        event.widget.config(bg="red")
        return False


def update_entry_value(entry: tk.Entry, value: float | str, digits: int = 2) -> None:
    "updates the entry with a float value, formatted to the specified number of digits is not selected"
    try:
        if entry.focus_get() == entry:
            return

        output: str = ""
        if isinstance(value, str):
            try:
                parsed_float = float(value)
                output = f"{parsed_float:.{digits}f}"
            except ValueError:
                output = value

        elif isinstance(value, (int, float)):
            output = f"{value:.{digits}f}"

        entry.delete(0, tk.END)
        entry.insert(0, output)
    except Exception as e:
        print(f"Error updating entry: {e}")


def bind_defaults_to_entry(
    escape_focus_target: tk.Widget,
    entry: tk.Entry,
    return_func: Optional[Callable[[tk.Event], None]] = None,
) -> None:
    """
    Bind default behavior to a tk.Entry widget.

    - <Return>: Triggers the given callback, if provided.
    - <Escape>: Restores focus to the parent widget.
    - <KeyRelease>: Runs real-time float validation (check_float_entry).

    Args:
        parent: The parent widget to focus when Escape is pressed.
        entry: The entry widget to bind events to.
        return_func: An optional function to call on Return key.
    """
    entry.bind("<KeyRelease>", check_float_entry)
    entry.bind("<Escape>", lambda e: escape_focus_target.focus_set())
    if return_func is not None:
        entry.bind(
            "<Return>", lambda e: (return_func(e), escape_focus_target.focus_set())
        )


def labeled_entry(
    parent: tk.Widget,
    callback: Optional[Callable[[tk.Event], None]],
    row: int,
    column: int,
    label_text: Optional[str] = None,
    escape_focus_target: Optional[tk.Widget] = None,
) -> tk.Entry:
    """Create a labeled entry field with a callback for the Return key.

    - <Return>: Triggers the given callback, if provided.
    - <Escape>: Restores focus to the parent widget.
    - <KeyRelease>: Runs real-time float validation (check_float_entry).

    Args:
        parent: Parent widget where the entry is placed.
        label_text: Text to display next to the entry field.
        callback: Function to call when the Return key is pressed.
        row: Row index in the grid.
        column: Column index in the grid
        escape_focus_target: Widget to focus when Escape is pressed.
    Returns:
        The created tk.Entry instance.
    """
    if label_text is not None:
        label = tk.Label(parent, text=label_text, bg="lightgray", width=15)
        label.grid(row=row, column=column, padx=2, pady=2, sticky="e")
        column = column + 1
    field = tk.Entry(parent, width=15)
    field.grid(row=row, column=column, padx=2, pady=2, sticky="w")
    bind_defaults_to_entry(
        escape_focus_target=(
            parent if escape_focus_target is None else escape_focus_target
        ),
        entry=field,
        return_func=callback,
    )
    return field


def readout_label(
    parent: tk.Widget,
    row: int,
    column: int,
    label_text: str = "---",
) -> tk.Label:
    """Create a label and a corresponding readout display.
    Args:
        parent: Parent widget where the label is placed.
        label_text: Text to display next to the readout field.
        row: Row index in the grid.
        column: Column index in the grid.
    Returns:
        The created tk.Label instance for the readout."""
    label = tk.Label(parent, text=label_text, bg="lightgray", width=15)
    label.grid(row=row, column=column, padx=2, pady=2, sticky="e")
    readout = tk.Label(parent, text="---", bg="lightgray", width=15)
    readout.grid(row=row, column=column + 1, padx=2, pady=2, sticky="w")
    return readout


def button(
    parent: tk.Widget,
    text: str,
    row: int,
    column: int,
    command: Callable[[tk.Event], None],
    state="normal",
) -> tk.Button:
    """
    Create a button with a label and attach it to a grid location.

    Args:
        parent: Parent widget where the button is placed.
        text: Label text shown on the button.
        row: Row index in the grid.
        column: Column index in the grid.
        command: Function to call when the button is pressed.
    Returns:
        The created tk.Button instance.
    """
    btn = tk.Button(parent, text=text, width=15, state=state)
    btn.grid(row=row, column=column, padx=2, pady=2, sticky="e")
    btn.config(command=command)
    return btn


def send_value(parent: tk.Widget, param: str, event: tk.Event | None) -> None:
    """Attempt to send the value from the event to the station. Show error on failure."""
    try:
        parent.focus_set()
        new_value = float(event.widget.get())
        station.set(param, new_value)
    except ValueError:
        messagebox.showerror("Invalid Value", f"Invalid value: {event.widget.get()}")


class PlotData:
    """Stores and manages historical data for plotting"""

    def __init__(self, length: int) -> None:
        """Initialize the data lists with a specified length."""
        self.length: int = length
        self.reset()

    def reset(self) -> None:
        """Reset all data lists to their initial state."""
        self.sp: List[float] = [0.0] * self.length
        self.pv: List[float] = [0.0] * self.length
        self.op: List[float] = [0.0] * self.length
        self.t: List[float] = [0.0] * self.length

    def append(self, pv: float, sp: float, op: float, t: float) -> None:
        """Append new data points to the lists, maintaining the specified length."""
        for attr, val in zip((self.sp, self.pv, self.op, self.t), (sp, pv, op, t)):
            attr.append(val)
            if len(attr) > self.length:
                attr.pop(0)

    def set_length(self, new_length: int) -> None:
        """Set a new length for the data lists, trims or append zeros to fill"""
        if new_length > 0:
            for attr in (self.sp, self.pv, self.op, self.t):
                if new_length > self.length:
                    attr[:0] = [0.0] * (new_length - self.length)
                else:
                    del attr[:-new_length]
            self.length = new_length


class PlotFrame:
    """Creates and updates the plot canvas embedded in a tkinter Frame."""

    def __init__(self, parent: tk.Widget) -> None:
        # plot frame returned
        self.frame = tk.Frame(parent, bg="white", relief="solid", bd=1)
        self.frame.grid(row=0, column=0, sticky="nsew", padx=(5, 10), pady=10)
        self.frame.columnconfigure(0, weight=1)
        self.frame.rowconfigure(0, weight=1)

        # build plot
        fig = Figure()
        self.axis_pv_sp: Axes = fig.add_subplot(111)
        self.axis_pv_sp.set_xlabel("Time [s]")
        self.axis_pv_sp.set_ylabel("PV / SP")
        self.axis_pv_sp.grid()

        self.axis_op: Axes = self.axis_pv_sp.twinx()
        self.axis_op.set_ylabel("OP", color="blue")
        self.axis_op.tick_params(axis="y", colors="blue")

        def create_line(ax: Axes, label: str, color: str) -> plt.Line2D:
            (line,) = ax.plot([], [], label=label)
            line.set_color(color)
            return line

        self.lines: Dict[str, plt.Line2D] = {
            "sp": create_line(self.axis_pv_sp, "Set", "green"),
            "pv": create_line(self.axis_pv_sp, "Meas", "red"),
            "op": create_line(self.axis_op, "Out", "blue"),
        }

        # build canvas on frame
        self.canvas = FigureCanvasTkAgg(fig, master=self.frame)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.grid(row=0, column=0, sticky="nsew")

    def set_plot_data(self, data: PlotData, limits: Dict[str, float]) -> None:
        """Update the plot with new data and adjust the axes limits.

        Args:
            data (PlotData): The data object containing the time, setpoint, process variable, and output values.
            limits (Dict[str, float]): ['pvsp_min', 'pvsp_max', 'op_min', 'op_max']
        """
        self.lines["sp"].set_data(data.t, data.sp)
        self.lines["pv"].set_data(data.t, data.pv)
        self.lines["op"].set_data(data.t, data.op)

        self.axis_pv_sp.set_ylim(limits["pvsp_min"], limits["pvsp_max"])
        self.axis_op.set_ylim(limits["op_min"], limits["op_max"])

        self.axis_pv_sp.relim()
        self.axis_op.relim()
        self.axis_pv_sp.autoscale_view()
        self.axis_op.autoscale_view()
        self.canvas.draw()


class PlotController:
    """Handles the logic for updating and configuring a real-time data plot."""

    def __init__(self, parent: tk.Widget) -> None:
        self.parent = parent
        self.data = PlotData(length=100)  # default length
        self.plot = PlotFrame(parent)

        self.scan_rate: int = 200
        self.limits: Dict[str, float] = {
            "pvsp_min": 0.0,
            "pvsp_max": 500.0,
            "op_min": 0.0,
            "op_max": 1.0,
        }

    def apply_default_scale(self, mode: str) -> None:
        """Set the default scale for the plot based on the mode (Temperature or Voltage)."""
        if mode == "T":
            self.limits["pvsp_min"] = 0
            self.limits["pvsp_max"] = 500
            self.plot.axis_pv_sp.set_ylabel("PV / SP [C°]")
        elif mode == "V":
            self.limits["pvsp_min"] = 0
            self.limits["pvsp_max"] = 10000
            self.plot.axis_pv_sp.set_ylabel("PV / SP [uV]")

    def write_data_to_plot(self) -> None:
        """Update the plot with the current data and limits."""
        self.plot.set_plot_data(self.data, self.limits)

    def append_data_point(self, sp: float, pv: float, op: float, t: float) -> None:
        """Append a new data point to the plot and update the display."""
        self.data.append(pv, sp, op, t)
        self.write_data_to_plot()

    def reset(self) -> None:
        """Reset the plot data and clear the display."""
        self.data.reset()
        self.write_data_to_plot()

    def get_frame(self) -> tk.Frame:
        """Return the frame containing the plot."""
        return self.plot.frame

    def open_config_window(self) -> None:
        """Open a configuration window for setting plot parameters."""
        config_window = tk.Toplevel(self.plot.frame)
        config_window.title("Plot Configuration")

        # Create all entry widgets first (do NOT populate yet)
        record_length_entry = labeled_entry(
            config_window,
            label_text="Points:",
            callback=lambda e: check_and_apply(),
            row=0,
            column=0,
        )

        scan_rate_entry = labeled_entry(
            config_window,
            label_text="Scan Rate [ms]:",
            callback=lambda e: check_and_apply(),
            row=1,
            column=0,
        )

        pvsp_limit_min_entry = labeled_entry(
            config_window,
            label_text="PV/SP Y (min/max):",
            callback=lambda e: check_and_apply(),
            row=2,
            column=0,
        )

        pvsp_limit_max_entry = labeled_entry(
            config_window,
            label_text=None,
            callback=lambda e: check_and_apply(),
            row=2,
            column=2,
        )

        op_limit_min_entry = labeled_entry(
            config_window,
            label_text="OP Y (min/max):",
            callback=lambda e: check_and_apply(),
            row=3,
            column=0,
        )

        op_limit_max_entry = labeled_entry(
            config_window,
            label_text=None,
            callback=lambda e: check_and_apply(),
            row=3,
            column=2,
        )

        def check_and_apply() -> None:
            """Check the entries and apply the values if valid."""
            entries = {
                "Record Length": (record_length_entry.get(), int),
                "Scan Rate": (scan_rate_entry.get(), int),
                "PV-SP min": (pvsp_limit_min_entry.get(), float),
                "PV-SP max": (pvsp_limit_max_entry.get(), float),
                "OP min": (op_limit_min_entry.get(), float),
                "OP max": (op_limit_max_entry.get(), float),
            }

            try:
                parsed = {key: cast(val) for key, (val, cast) in entries.items()}
            except ValueError:
                return  # skip apply if any value fails

            self.data.set_length(parsed["Record Length"])
            self.scan_rate = parsed["Scan Rate"]
            self.limits.update(
                {
                    "pvsp_min": parsed["PV-SP min"],
                    "pvsp_max": parsed["PV-SP max"],
                    "op_min": parsed["OP min"],
                    "op_max": parsed["OP max"],
                }
            )

            config_window.destroy()

        # Set focus explicitly to the config window (important!)
        config_window.focus_set()

        # Now safely update entries after window has the focus
        update_entry_value(record_length_entry, self.data.length, digits=0)
        update_entry_value(scan_rate_entry, self.scan_rate, digits=0)
        update_entry_value(pvsp_limit_min_entry, self.limits["pvsp_min"], digits=0)
        update_entry_value(pvsp_limit_max_entry, self.limits["pvsp_max"], digits=0)
        update_entry_value(op_limit_min_entry, self.limits["op_min"], digits=2)
        update_entry_value(op_limit_max_entry, self.limits["op_max"], digits=2)

        # Add Apply / Cancel buttons
        tk.Button(config_window, text="Apply", command=check_and_apply).grid(
            row=5, column=1, padx=5, pady=10, sticky="ew"
        )
        tk.Button(config_window, text="Cancel", command=config_window.destroy).grid(
            row=5, column=2, padx=5, pady=10, sticky="ew"
        )


class PIDtab(ttk.Frame):
    """
    Tab interface for configuring and visualizing PID control settings.
    Integrates a plot for real-time data and entry widgets for user input.
    """

    def __init__(self, parent):
        """Initialize the PID settings tab and layout all UI components."""
        super().__init__(parent)

        self.record_start_time = 0

        self.columnconfigure(0, weight=1)
        self.columnconfigure(1, weight=0)
        self.rowconfigure(0, weight=1)

        self.plot_controller = PlotController(self)
        self.plot_controller.get_frame().grid(row=0, column=0, sticky="nsew")

        self.controls_frame = tk.Frame(
            self, width=200, bg="#f0f0f0", relief="solid", bd=1
        )
        self.controls_frame.grid(row=0, column=1, sticky="ns", padx=(5, 10), pady=10)

        self.populate_control_frame()
        self.switch_to_setT()

        self.unit_mode = "T"

        self.tc_U_avg_len = 15  # average length for thermocouple average readout
        self.tc_U_avg = []

    def populate_control_frame(self):
        """Create and place all control elements (entries, buttons, readouts) in the control panel."""
        self.modeT_toggle_btn = button(
            self.controls_frame,
            text="Set Mode T[C°]",
            row=0,
            column=0,
            command=self.switch_to_setT,
            state="disabled",
        )
        self.modeV_toggle_btn = button(
            self.controls_frame,
            text="Set Mode V[uV]",
            row=0,
            column=1,
            command=self.switch_to_setV,
            state="disabled",
        )

        self.setpoint_entry = labeled_entry(
            self.controls_frame,
            label_text="Setpoint",
            callback=self.send_new_setpoint,
            row=1,
            column=0,
        )

        self.enable_btn = button(
            self.controls_frame,
            text="Turn ON",
            row=2,
            column=0,
            command=self.tip_state_toggled,
        )
        self.indicator = tk.Label(
            self.controls_frame, text="OFF", width=15, relief="solid", bg="red", fg="black"
        )
        self.indicator.grid(row=2, column=1, padx=2, pady=2)

        # Readouts
        tk.Label(self.controls_frame, text="Readouts", bg="lightgray").grid(
            row=3, column=0, columnspan=2, padx=2, pady=2, sticky="ew"
        )

        self.readout_pv = readout_label(
            self.controls_frame, label_text="PV", row=4, column=0
        )
        self.readout_op = readout_label(
            self.controls_frame, label_text="OP", row=5, column=0
        )
        self.readout_tc_U_avg = readout_label(
            self.controls_frame, label_text="TC U[uV] avg", row=6, column=0
        )

        # PID
        tk.Label(self.controls_frame, text="PID Parameters", bg="lightgray").grid(
            row=7, column=0, columnspan=2, padx=2, pady=2, sticky="ew"
        )

        self.pid_p_entry = labeled_entry(
            self.controls_frame,
            label_text="kp",
            callback=lambda event: (
                send_value(self, station.commands.pid_kp, event),
                update_entry_value(
                    self.pid_p_entry,
                    station.get(station.commands.pid_kp),
                    digits=5,
                ),
            ),
            row=8,
            column=0,
        )

        self.pid_i_entry = labeled_entry(
            self.controls_frame,
            label_text="kI [1/s]",
            callback=lambda event: (
                send_value(self, station.commands.pid_ki, event),
                update_entry_value(
                    self.pid_i_entry,
                    station.get(station.commands.pid_ki),
                    digits=5,
                ),
            ),
            row=9,
            column=0,
        )

        self.pid_d_entry = labeled_entry(
            self.controls_frame,
            label_text="kD [s]",
            callback=lambda event: (
                send_value(self, station.commands.pid_kd, event),
                update_entry_value(
                    self.pid_d_entry,
                    station.get(station.commands.pid_kd),
                    digits=5,
                ),
            ),
            row=10,
            column=0,
        )

        self.pid_dt_entry = labeled_entry(
            self.controls_frame,
            label_text="D-LPF tau",
            callback=lambda event: (
                send_value(self, station.commands.pid_derivative_filter_time, event),
                update_entry_value(
                    self.pid_dt_entry,
                    station.get(station.commands.pid_derivative_filter_time),
                ),
            ),
            row=11,
            column=0,
        )

        button(
            self.controls_frame,
            text="Plot Config",
            command=self.plot_controller.open_config_window,
            row=12,
            column=0,
        )

    def reset_plot(self):
        """Reset the plot data and start time."""
        self.record_start_time = time.time()
        self.plot_controller.reset()

    def reload(self):
        """Reset the data plot and start time."""
        self.reset_plot()
        update_entry_value(
            self.pid_p_entry, station.get(station.commands.pid_kp), digits=4
        )
        update_entry_value(
            self.pid_i_entry, station.get(station.commands.pid_ki), digits=4
        )
        update_entry_value(
            self.pid_d_entry, station.get(station.commands.pid_kd), digits=4
        )
        update_entry_value(
            self.pid_dt_entry,
            station.get(station.commands.pid_derivative_filter_time),
        )

    def switch_to_setT(self):
        """Switch the mode to temperature (C°) and update the plot settings."""
        self.unit_mode = "T"
        self.plot_controller.plot.axis_pv_sp.set_ylabel("PV / SP [C°]")
        self.plot_controller.apply_default_scale("T")
        self.modeT_toggle_btn.config(state="disabled")
        self.modeV_toggle_btn.config(state="normal")
        self.reset_plot()

    def switch_to_setV(self):
        """Switch the mode to voltage (uV) and update the plot settings."""
        self.unit_mode = "V"
        self.plot_controller.plot.axis_pv_sp.set_ylabel("PV / SP [uV]")
        self.plot_controller.apply_default_scale("V")
        self.modeT_toggle_btn.config(state="normal")
        self.modeV_toggle_btn.config(state="disabled")
        self.reset_plot()

    def send_new_setpoint(self, event: tk.Event):
        if self.unit_mode == "T":
            send_value(self, station.commands.temp_set, event)
        else:
            send_value(self, station.commands.tc_voltage_setpoint, event)

    def tip_state_toggled(self):
        """Toggle the state of the station (ON/OFF) and update the button text."""
        new_state = False
        if self.enable_btn.cget("text") == "Turn ON":
            new_state = True
        station.set(station.commands.enable, int(new_state))

    def acquire_datapoint(self):
        """Poll values from the station and update UI elements and plot."""
        if station.tip_index is None:
            return self.plot_controller.scan_rate

        if station.tip_changed:
            self.reload()

        # get the current values from the station
        tc_U = float(station.get(station.commands.tc_voltage_measure))

        if self.unit_mode == "T":
            sp = float(station.get(station.commands.temp_set))
            pv = float(station.get(station.commands.temp_measure))
        else:
            sp = float(station.get(station.commands.tc_voltage_setpoint))
            pv = tc_U

        en = station.get(station.commands.enable) == "1"
        sleep = station.get(station.commands.sleep_state) == "1"
        op = float(station.get(station.commands.pid_op))
        t = time.time() - self.record_start_time

        # avg
        self.tc_U_avg.append(tc_U)
        if len(self.tc_U_avg) > self.tc_U_avg_len:
            self.tc_U_avg.pop(0)
        avg_U = sum(self.tc_U_avg) / len(self.tc_U_avg)

        # append data poinrt to plot
        self.plot_controller.append_data_point(sp, pv, op, t)

        # update UI elements
        update_entry_value(self.setpoint_entry, sp, digits=1)

        self.enable_btn.config(text="Turn OFF" if en else "Turn ON")

        if sleep:
            self.indicator.config(text="Sleep", bg="yellow")
        else:
            self.indicator.config(
                text="ON" if en else "OFF", bg="green" if en else "red"
            )

        self.readout_op.config(text=f"{op:.4f}")
        self.readout_pv.config(text=f"{pv:.2f}")
        self.readout_tc_U_avg.config(text=f"{avg_U:.2f}")

        return self.plot_controller.scan_rate


class SerialComTab(ttk.Frame):
    def __init__(self, parent):
        super().__init__(parent)

        self.on_connect = None
        self.on_disconnect = None

        self.default_tip = next(iter(station.tips.values()))
        station.tip_index = self.default_tip

        self.build()

    def build(self):
        """Set up the connection controls and tip selector UI."""
        self.com_port_combo = ttk.Combobox(self, values=[], width=15)

        self.com_port_combo.grid(row=0, column=0, padx=10, pady=10)

        self.connect_toggle_btn = tk.Button(
            self, text="Connect", width=15, command=self.connect_toggle
        )

        self.connect_toggle_btn.grid(row=0, column=1, padx=10, pady=10)

        self.indicator = tk.Label(
            self, text="Disconnected", width=15, relief="solid", bg="red", fg="black"
        )

        self.indicator.grid(row=0, column=2, padx=10, pady=10)

        tk.Label(self, text="Target tip", bg="lightgray", width=15).grid(
            row=1, column=0, padx=10, pady=10, sticky="e"
        )

        self.tip_entry = ttk.Combobox(self, width=15, values=list(station.tips.keys()))

        initial_key = next(k for k, v in station.tips.items() if v == self.default_tip)

        self.tip_entry.set(initial_key)
        self.tip_entry.bind("<<ComboboxSelected>>", self.update_target_tip)
        self.tip_entry.grid(row=1, column=1, padx=10, pady=10, sticky="w")

    def on_connect_do(self, command):
        self.on_connect = command

    def on_disconnect_do(self, command):
        self.on_disconnect = command

    def update_available_coms(self):
        """Refresh the available COM ports and update the connection indicator."""
        self.com_port_combo.configure(values=[i.name for i in list(comports())])

        self.indicator.config(bg="green" if station.connected else "red")

        self.indicator.configure(
            text="Connected" if station.connected else "Disconnected"
        )
        return 500  # ms to next update

    def update_target_tip(self, event: tk.Event) -> None:
        """Change the target tip based on user selection."""
        station.tip_changed = station.set_target_tip(event.widget.get())

    def connect_toggle(self):
        """Toggle the connection state."""
        if station.connected:
            self.disconnect()
        else:
            self.connect()

    def disconnect(self):
        """Disconnect from the station."""
        station.disconnect()
        self.connect_toggle_btn.config(text="Connect")
        if self.on_disconnect is not None:
            self.on_disconnect()

    def connect(self) -> None:
        """Connect to the station."""
        target_port = self.com_port_combo.get()
        success = station.connect(target_port)
        if not success:
            return
        self.connect_toggle_btn.config(text="Disconnect")
        if self.on_connect is not None:
            self.on_connect()


def default_values_config_window(parent, message: str):
    """
    Spawn a warning dialog with reset warning message, entry field, and OK/Abort buttons.

    Args:
        parent: The parent window for the dialog.
        message: The warning text to display.
        on_ok: A function that takes one argument (the entry value) and is called if OK is pressed.
    """
    window = tk.Toplevel(parent)
    window.title("Warning")
    window.grab_set()  # Make it modal
    window.resizable(False, False)

    # Message label
    tk.Label(window, text=message, fg="red", font=("Arial", 10, "bold")).grid(
        row=0, column=0, columnspan=2, padx=20, pady=(10, 5), sticky="w"
    )

    tk.Label(
        window,
        text="please insert thermocouple contant S[uV/K]",
        font=("Arial", 10, "bold"),
    ).grid(row=1, column=0, columnspan=2, padx=20, pady=(10, 5), sticky="w")

    
    def return_pressed(event: tk.Event) -> None:
        send_value(parent, station.commands.restore_default_config, event)
        window.destroy()

    value_entry= labeled_entry(window, label_text="TC S[uV/K]", callback=return_pressed, row=2, column=0, escape_focus_target=parent)

    def ok_pressed(entry: tk.Entry) -> None:
        """Handle the OK button press."""
        value = float(entry.get())
        station.set(station.commands.restore_default_config, value)
        window.destroy()

    tk.Button(window, text="OK", command=lambda: ok_pressed(entry = value_entry), width=10).grid(
        row=3, column=0, padx=5, pady=5
    )
    tk.Button(window, text="Abort", command=window.destroy, width=10).grid(
        row=3, column=1, padx=5, pady=5
    )


class DataRow:
    def __init__(self, parent, index: int, row: int):
        tk.Label(parent, text=index, bg="lightgray").grid(
            row=row, column=0, padx=2, pady=1
        )

        self.index = index

        self.tc_v_entry = tk.Entry(parent, width=15)
        self.tc_v_entry.grid(row=row, column=1, padx=2, pady=1)
        bind_defaults_to_entry(
            parent, self.tc_v_entry, lambda e: self.send_edited_value()
        )

        self.tc_s_entry = tk.Entry(parent, width=15)
        self.tc_s_entry.grid(row=row, column=2, padx=2, pady=1)
        bind_defaults_to_entry(
            parent, self.tc_s_entry, lambda e: self.send_edited_value()
        )

    def update_values(self, v: float, s: float):
        """Update the displayed values in the entry fields."""
        update_entry_value(self.tc_v_entry, v)
        update_entry_value(self.tc_s_entry, s)

    def send_edited_value(self) -> None:
        """Send the edited values to the station."""
        try:
            v = float(self.tc_v_entry.get())
            s = float(self.tc_s_entry.get())
            station.set(station.commands.cal_tc_table, f"{self.index}[{v},{s}]")
            # reload
            data_str = station.get(station.commands.cal_tc_table, self.index)
            data_str = data_str[1:-1]  # remove parenthesis
            comma_pos = data_str.find(",")
            v = float(data_str[:comma_pos])
            s = float(data_str[comma_pos + 1 :])
            self.update_values(v, s)
        except ValueError:
            print("Invalid value found")


class TcCalFrame(ttk.Frame):
    def __init__(self, parent: tk.Widget):
        """Create a frame with a 3-column table of thermocouple calibration rows."""
        style = ttk.Style()
        style.configure("TcCal.TFrame", background="lightgray")

        super().__init__(parent, style="TcCal.TFrame")

        tk.Label(self, text="Index", font=("Arial", 10, "bold"), bg="lightgray").grid(
            row=0, column=0, padx=2, pady=(0, 4)
        )

        tk.Label(
            self, text="Junction dV [uV]", font=("Arial", 10, "bold"), bg="lightgray"
        ).grid(row=0, column=1, padx=2, pady=(0, 4))

        tk.Label(
            self, text="Hot Junction T [°C]", font=("Arial", 10, "bold"), bg="lightgray"
        ).grid(row=0, column=2, padx=2, pady=(0, 4))

        self.data_rows: list[DataRow] = []
        rows = int(station.get(station.commands.cal_tc_table))
        for index in range(rows):
            self.data_rows.append(DataRow(self, index, index + 1))

    def reload(self) -> None:
        """Reload the calibration table data from the station."""
        for index, row in enumerate(self.data_rows):
            try:
                data_str = station.get(station.commands.cal_tc_table, index)
                data_str = data_str[1:-1]  # remove parenthesis
                comma_pos = data_str.find(",")
                v = float(data_str[:comma_pos])
                s = float(data_str[comma_pos + 1 :])
                row.update_values(v, s)
            except Exception as e:
                print(f"Error updating row {index}: {e}")


class DeviceInfoTab(ttk.Frame):
    """Tab for displaying and editing device configuration and calibration table."""

    def __init__(self, parent: tk.Widget):
        """
        Initialize the device info tab.

        Args:
            parent: The parent widget (typically a notebook or main window).
        """
        super().__init__(parent)

    def build(self) -> None:
        """Construct and place UI elements within the tab."""
        self.t_sp_min = labeled_entry(
            self,
            label_text="T set [C°] MIN",
            callback=lambda event: (
                send_value(self, station.commands.temp_set_min, event),
                update_entry_value(
                    self.t_sp_min, station.get(station.commands.temp_set_min)
                ),
            ),
            row=0,
            column=0,
        )
        self.t_sp_max = labeled_entry(
            self,
            label_text="T set [C°] MAX",
            callback=lambda event: (
                send_value(self, station.commands.temp_set_max, event),
                update_entry_value(
                    self.t_sp_max, station.get(station.commands.temp_set_max)
                ),
            ),
            row=1,
            column=0,
        )
        self.t_runaway = labeled_entry(
            self,
            label_text="runaway det. [C°]",
            callback=lambda event: (
                send_value(self, station.commands.temp_runaway, event),
                update_entry_value(
                    self.t_runaway, station.get(station.commands.temp_runaway)
                ),
            ),
            row=2,
            column=0,
        )
        self.sleep_delay = labeled_entry(
            self,
            label_text="Sleep delay [ms]",
            callback=lambda event: (
                send_value(self, station.commands.sleep_delay, event),
                update_entry_value(
                    self.sleep_delay, station.get(station.commands.sleep_delay)
                ),
            ),
            row=3,
            column=0,
        )
        self.sleep_temp = labeled_entry(
            self,
            label_text="Sleep Temp [C°]",
            callback=lambda event: (
                send_value(self, station.commands.sleep_temp, event),
                update_entry_value(
                    self.sleep_temp, station.get(station.commands.sleep_temp)
                ),
            ),
            row=4,
            column=0,
        )

        self.cal_table = TcCalFrame(self)
        self.cal_table.grid(row=5, column=0, columnspan=2, padx=2, pady=2)

        # col 3
        tk.Button(
            self,
            text="Reset Configs",
            command=lambda: (
                default_values_config_window(
                    self, "this will reset tuning, limits and tc_cal"
                ),
                self.reload(),
            ),
        ).grid(row=0, rowspan=2, column=3, padx=10, pady=10)

    def reload(self) -> None:
        """Update all displayed values from the station."""
        update_entry_value(self.t_sp_min, station.get(station.commands.temp_set_min))
        update_entry_value(self.t_sp_max, station.get(station.commands.temp_set_max))
        update_entry_value(self.t_runaway, station.get(station.commands.temp_runaway))
        update_entry_value(self.sleep_delay, station.get(station.commands.sleep_delay))
        update_entry_value(self.sleep_temp, station.get(station.commands.sleep_temp))

        self.cal_table.reload()


class JBcloneTuner:
    def __init__(self, root_window: tk.Tk):
        self.root_window = root_window
        self.root_window.title("JBClone Tuning Utility")
        self.root_window.geometry("800x600")

        style = ttk.Style()
        style.theme_use("default")
        style.configure(
            "TNotebook.Tab", padding=[10, 5], font=("Helvetica", 10, "bold")
        )

        self.station_last_conection_state = False

        self.build()
        self.update_loop()

    def on_connect(self):
        """Callback function to be executed when a connection is established."""
        try:
            self.device_tab.build()
            self.pid_tab.reload()
            self.notebook.tab(self.pid_tab_index, state="normal")
            self.notebook.tab(self.device_tab_index, state="normal")

            # load data
            self.device_tab.reload()
            self.root_window.after(50, lambda: self.notebook.focus_set())
        except Exception as e:
            print(e)
            self.connection_tab.disconnect()

    def on_disconnect(self):
        """Callback function to be executed when a connection is lost."""
        self.notebook.tab(self.pid_tab_index, state="disabled")
        self.notebook.tab(self.device_tab_index, state="disabled")
        self.notebook.select(self.connection_tab_index)

    def build(self):
        """Set up the main window and its components."""
        self.notebook = ttk.Notebook(self.root_window)
        self.notebook.pack(expand=1, fill="both")

        self.connection_tab = SerialComTab(self.notebook)
        self.connection_tab.on_disconnect_do(lambda: self.on_disconnect())
        self.connection_tab.on_connect_do(lambda: self.on_connect())
        self.notebook.add(self.connection_tab, text="Connection")
        self.connection_tab_index = self.notebook.index(self.connection_tab)

        self.pid_tab = PIDtab(self.notebook)
        self.notebook.add(self.pid_tab, text="PID Settings")
        self.pid_tab_index = self.notebook.index(self.pid_tab)
        self.notebook.tab(self.pid_tab_index, state="disabled")

        self.device_tab = DeviceInfoTab(self.notebook)
        self.notebook.add(self.device_tab, text="Device Info")
        self.device_tab_index = self.notebook.index(self.device_tab)
        self.notebook.tab(self.device_tab_index, state="disabled")

    def update_loop(self):
        """Main loop for updating the UI and acquiring data from the station."""
        update_delay = 1000
        update_delay = self.connection_tab.update_available_coms()
        if station.connected:
            update_delay = self.pid_tab.acquire_datapoint()

        # temp clear
        station.tip_changed = False

        # loop back
        self.root_window.after(update_delay, self.update_loop)


if __name__ == "__main__":
    station: Station = Station()
    root = tk.Tk()
    app = JBcloneTuner(root)
    root.mainloop()
