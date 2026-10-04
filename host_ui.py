# -*- coding: utf-8 -*-
"""
STM32 串口状态上位机
====================
功能：
  1. 打开/关闭串口，向 STM32(USART2, 115200 8N1) 发送查询命令。
  2. 接收并显示 STM32 返回的 LED / Buzzer / HC595 状态。

STM32 端返回格式（每行以 \r\n 结尾）：
  Recv OK
  LED:ON Buzzer:OFF HC595:LED3
或
  LED:ON Buzzer:OFF HC595:NONE

依赖：pyserial（安装命令：pip install pyserial）
"""

import re
import queue
import threading
import tkinter as tk
from tkinter import ttk, messagebox

import serial
import serial.tools.list_ports


class SerialHost:
    def __init__(self, root):
        self.root = root
        root.title("STM32 状态上位机")
        root.geometry("560x480")

        self.ser = None
        self.rx_queue = queue.Queue()
        self.read_thread = None
        self.running = False

        self._build_ui()
        self._refresh_ports()
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        self.root.after(80, self._poll_queue)

    def _build_ui(self):
        # ---------- 串口设置 ----------
        frm = ttk.LabelFrame(self.root, text="串口设置")
        frm.pack(fill="x", padx=8, pady=4)

        ttk.Label(frm, text="串口:").grid(row=0, column=0, padx=4, pady=6)
        self.port_var = tk.StringVar()
        self.port_cb = ttk.Combobox(frm, textvariable=self.port_var, width=12)
        self.port_cb.grid(row=0, column=1, padx=4, pady=6)

        ttk.Label(frm, text="波特率:").grid(row=0, column=2, padx=4, pady=6)
        self.baud_var = tk.StringVar(value="115200")
        ttk.Combobox(frm, textvariable=self.baud_var, width=10,
                     values=["9600", "115200", "256000"]).grid(row=0, column=3, padx=4, pady=6)

        self.open_btn = ttk.Button(frm, text="打开串口", command=self.toggle_port)
        self.open_btn.grid(row=0, column=4, padx=8, pady=6)

        ttk.Button(frm, text="刷新", command=self._refresh_ports).grid(row=0, column=5, padx=4, pady=6)

        # ---------- 状态显示 ----------
        state = ttk.LabelFrame(self.root, text="设备状态")
        state.pack(fill="x", padx=8, pady=4)

        self.led_lbl = tk.Label(state, text="LED: --", font=("Microsoft YaHei", 13, "bold"),
                                fg="gray", width=18, anchor="w")
        self.led_lbl.grid(row=0, column=0, padx=8, pady=8)

        self.buzzer_lbl = tk.Label(state, text="Buzzer: --", font=("Microsoft YaHei", 13, "bold"),
                                   fg="gray", width=18, anchor="w")
        self.buzzer_lbl.grid(row=0, column=1, padx=8, pady=8)

        self.hc595_lbl = tk.Label(state, text="HC595: --", font=("Microsoft YaHei", 13, "bold"),
                                  fg="gray", width=22, anchor="w")
        self.hc595_lbl.grid(row=0, column=2, padx=8, pady=8)

        # ---------- 发送区 ----------
        send = ttk.LabelFrame(self.root, text="发送命令（发送 State 查询状态）")
        send.pack(fill="x", padx=8, pady=4)
        self.send_var = tk.StringVar(value="State")
        ttk.Entry(send, textvariable=self.send_var).pack(side="left", fill="x", expand=True, padx=8, pady=6)
        ttk.Button(send, text="发送", command=self.send_query).pack(side="left", padx=8, pady=6)

        # ---------- 接收日志 ----------
        log = ttk.LabelFrame(self.root, text="接收日志")
        log.pack(fill="both", expand=True, padx=8, pady=4)
        self.log_text = tk.Text(log, height=12, state="disabled")
        self.log_text.pack(fill="both", expand=True, padx=4, pady=4)

    # ---------- 串口操作 ----------
    def _refresh_ports(self):
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_cb["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])

    def toggle_port(self):
        if self.ser and self.ser.is_open:
            self.close_port()
        else:
            self.open_port()

    def open_port(self):
        port = self.port_var.get().strip()
        if not port:
            messagebox.showwarning("提示", "请选择串口")
            return
        try:
            self.ser = serial.Serial(port, int(self.baud_var.get()), timeout=0.2)
        except Exception as e:
            messagebox.showerror("错误", f"打开串口失败: {e}")
            return

        self.running = True
        self.read_thread = threading.Thread(target=self._read_loop, daemon=True)
        self.read_thread.start()
        self.open_btn.config(text="关闭串口")
        self._log(f"已打开 {port} @ {self.baud_var.get()}")

    def close_port(self):
        self.running = False
        if self.read_thread:
            self.read_thread.join(timeout=1)
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
            self.ser = None
        self.open_btn.config(text="打开串口")
        self._log("已关闭串口")

    def send_query(self):
        if not (self.ser and self.ser.is_open):
            messagebox.showwarning("提示", "请先打开串口")
            return
        text = self.send_var.get()
        try:
            self.ser.write((text + "\r\n").encode("utf-8"))
            self._log(f">> {text}")
        except Exception as e:
            self._log(f"发送失败: {e}")

    def on_close(self):
        self.close_port()
        self.root.destroy()

    # ---------- 数据接收 ----------
    def _read_loop(self):
        buf = b""
        while self.running:
            try:
                if self.ser.in_waiting:
                    buf += self.ser.read(self.ser.in_waiting)
                    lines = buf.split(b"\r\n")
                    buf = lines[-1]
                    for line in lines[:-1]:
                        self.rx_queue.put(line.decode("utf-8", errors="replace"))
            except Exception as e:
                self.rx_queue.put(f"[读取错误] {e}")
                break

    def _poll_queue(self):
        while not self.rx_queue.empty():
            line = self.rx_queue.get_nowait()
            self._handle_line(line)
        self.root.after(80, self._poll_queue)

    def _handle_line(self, line):
        line = line.strip()
        if not line:
            return
        if line == "Read Fail":
            self._log("<< 读取失败")
            return
        self._log(f"<< {line}")
        self._parse_status(line)

    # ---------- 状态解析 ----------
    def _parse_status(self, line):
        if not line.startswith("LED:"):
            return
        m = re.match(r"LED:(?P<led>ON|OFF) Buzzer:(?P<buz>ON|OFF) HC595:(?P<hc>LED\d+|NONE)", line)
        if not m:
            return

        led = m.group("led")
        buz = m.group("buz")
        hc = m.group("hc")

        self._set_state(self.led_lbl, "LED", led)
        self._set_state(self.buzzer_lbl, "Buzzer", buz)

        if hc == "NONE":
            self.hc595_lbl.config(text="HC595: 无灯亮", fg="gray")
        else:
            self.hc595_lbl.config(text=f"HC595: 第{hc[3:]}个灯亮", fg="green")

    def _set_state(self, lbl, name, state):
        if state == "ON":
            lbl.config(text=f"{name}: 开", fg="green")
        else:
            lbl.config(text=f"{name}: 关", fg="red")

    # ---------- 日志 ----------
    def _log(self, text):
        self.log_text.config(state="normal")
        self.log_text.insert("end", text + "\n")
        self.log_text.see("end")
        self.log_text.config(state="disabled")


def main():
    root = tk.Tk()
    SerialHost(root)
    root.mainloop()


if __name__ == "__main__":
    main()
