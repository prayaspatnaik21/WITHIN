#!/usr/bin/env python3
"""
Interactive two-image comparison viewer.

Example:
    python visualization/image_slider_viewer.py \
        --left isp_output.png \
        --right segmentation_output.png \
        --left-name "ISP output" \
        --right-name "Segmentation output"
"""

from __future__ import annotations

import argparse
import sys
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox

try:
    from PIL import Image, ImageTk
except ImportError as exc:
    raise SystemExit(
        "This viewer needs Pillow for image loading and resizing.\n"
        "Install it with: python -m pip install pillow"
    ) from exc


CANVAS_BACKGROUND = "#16181d"
PANEL_BACKGROUND = "#20242b"
TEXT_COLOR = "#f2f4f8"
MUTED_TEXT_COLOR = "#aab2c0"
ACCENT_COLOR = "#4da3ff"
SLIDER_COLOR = "#ffffff"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Open a draggable before/after image comparison UI."
    )
    parser.add_argument("--left", type=Path, help="Path to the left image.")
    parser.add_argument("--right", type=Path, help="Path to the right image.")
    parser.add_argument(
        "--left-name",
        default="Left image",
        help='Label for the left image, for example "ISP output".',
    )
    parser.add_argument(
        "--right-name",
        default="Right image",
        help='Label for the right image, for example "Segmentation output".',
    )
    return parser.parse_args()


def load_image(path: Path) -> Image.Image:
    return Image.open(path).convert("RGB")


class ImageSliderViewer:
    def __init__(
        self,
        root: tk.Tk,
        left_path: Path | None,
        right_path: Path | None,
        left_name: str,
        right_name: str,
    ) -> None:
        self.root = root
        self.root.title("Image Slider Comparison")
        self.root.geometry("1100x760")
        self.root.minsize(720, 520)
        self.root.configure(bg=PANEL_BACKGROUND)

        self.left_path = left_path
        self.right_path = right_path
        self.left_image: Image.Image | None = None
        self.right_image: Image.Image | None = None
        self.display_left: Image.Image | None = None
        self.display_right: Image.Image | None = None
        self.tk_image: ImageTk.PhotoImage | None = None
        self.image_x = 0
        self.image_width = 1
        self.slider_ratio = 0.5
        self.dragging = False

        self.left_name_var = tk.StringVar(value=left_name)
        self.right_name_var = tk.StringVar(value=right_name)
        self.status_var = tk.StringVar(value="Choose two images to compare.")

        self._build_controls()
        self._build_canvas()
        self._bind_events()

        if self.left_path and self.right_path:
            self._load_pair(self.left_path, self.right_path)

    def _build_controls(self) -> None:
        controls = tk.Frame(self.root, bg=PANEL_BACKGROUND, padx=14, pady=12)
        controls.pack(fill=tk.X)

        left_button = tk.Button(
            controls,
            text="Choose left image",
            command=self._choose_left_image,
            bg="#2d333d",
            fg=TEXT_COLOR,
            activebackground="#38414f",
            activeforeground=TEXT_COLOR,
            relief=tk.FLAT,
            padx=12,
            pady=7,
        )
        left_button.grid(row=0, column=0, sticky="w")

        right_button = tk.Button(
            controls,
            text="Choose right image",
            command=self._choose_right_image,
            bg="#2d333d",
            fg=TEXT_COLOR,
            activebackground="#38414f",
            activeforeground=TEXT_COLOR,
            relief=tk.FLAT,
            padx=12,
            pady=7,
        )
        right_button.grid(row=0, column=1, sticky="w", padx=(10, 20))

        tk.Label(controls, text="Left name", bg=PANEL_BACKGROUND, fg=MUTED_TEXT_COLOR).grid(
            row=0, column=2, sticky="e", padx=(0, 8)
        )
        left_name = tk.Entry(
            controls,
            textvariable=self.left_name_var,
            bg="#15181e",
            fg=TEXT_COLOR,
            insertbackground=TEXT_COLOR,
            relief=tk.FLAT,
            width=22,
        )
        left_name.grid(row=0, column=3, sticky="ew")

        tk.Label(controls, text="Right name", bg=PANEL_BACKGROUND, fg=MUTED_TEXT_COLOR).grid(
            row=0, column=4, sticky="e", padx=(16, 8)
        )
        right_name = tk.Entry(
            controls,
            textvariable=self.right_name_var,
            bg="#15181e",
            fg=TEXT_COLOR,
            insertbackground=TEXT_COLOR,
            relief=tk.FLAT,
            width=22,
        )
        right_name.grid(row=0, column=5, sticky="ew")

        controls.columnconfigure(3, weight=1)
        controls.columnconfigure(5, weight=1)

        status = tk.Label(
            self.root,
            textvariable=self.status_var,
            bg=PANEL_BACKGROUND,
            fg=MUTED_TEXT_COLOR,
            anchor="w",
            padx=14,
        )
        status.pack(fill=tk.X)

    def _build_canvas(self) -> None:
        self.canvas = tk.Canvas(
            self.root,
            bg=CANVAS_BACKGROUND,
            highlightthickness=0,
            cursor="sb_h_double_arrow",
        )
        self.canvas.pack(fill=tk.BOTH, expand=True, padx=14, pady=14)

    def _bind_events(self) -> None:
        self.canvas.bind("<Configure>", lambda _event: self._render())
        self.canvas.bind("<ButtonPress-1>", self._start_drag)
        self.canvas.bind("<B1-Motion>", self._drag)
        self.canvas.bind("<ButtonRelease-1>", self._stop_drag)
        self.left_name_var.trace_add("write", lambda *_args: self._render())
        self.right_name_var.trace_add("write", lambda *_args: self._render())

    def _choose_left_image(self) -> None:
        path = self._ask_image_path()
        if not path:
            return

        self.left_path = path
        if self.right_path:
            self._load_pair(self.left_path, self.right_path)
        else:
            self.status_var.set("Left image selected. Choose a right image.")

    def _choose_right_image(self) -> None:
        path = self._ask_image_path()
        if not path:
            return

        self.right_path = path
        if self.left_path:
            self._load_pair(self.left_path, self.right_path)
        else:
            self.status_var.set("Right image selected. Choose a left image.")

    def _ask_image_path(self) -> Path | None:
        filename = filedialog.askopenfilename(
            title="Choose image",
            filetypes=[
                ("Image files", "*.png *.jpg *.jpeg *.bmp *.tif *.tiff"),
                ("All files", "*.*"),
            ],
        )
        return Path(filename) if filename else None

    def _load_pair(self, left_path: Path, right_path: Path) -> None:
        try:
            self.left_image = load_image(left_path)
            self.right_image = load_image(right_path)
        except OSError as exc:
            messagebox.showerror("Image load failed", str(exc))
            return

        self.status_var.set(
            f"Comparing {left_path.name} and {right_path.name}. Drag the divider."
        )
        self._render()

    def _start_drag(self, event: tk.Event) -> None:
        self.dragging = True
        self._update_slider(event.x)

    def _drag(self, event: tk.Event) -> None:
        if self.dragging:
            self._update_slider(event.x)

    def _stop_drag(self, event: tk.Event) -> None:
        self.dragging = False
        self._update_slider(event.x)

    def _update_slider(self, x_position: int) -> None:
        image_width = max(self.image_width, 1)
        self.slider_ratio = min(max((x_position - self.image_x) / image_width, 0.0), 1.0)
        self._render()

    def _render(self) -> None:
        self.canvas.delete("all")

        canvas_width = self.canvas.winfo_width()
        canvas_height = self.canvas.winfo_height()
        if canvas_width <= 1 or canvas_height <= 1:
            return

        if self.left_image is None or self.right_image is None:
            self._draw_empty_state(canvas_width, canvas_height)
            return

        self.display_left, image_x, image_y = self._fit_image(self.left_image, canvas_width, canvas_height)
        self.display_right, _, _ = self._fit_image(self.right_image, canvas_width, canvas_height)
        self.image_x = image_x
        self.image_width = self.display_left.width

        combined = self.display_right.copy()
        split_x = int(self.display_left.width * self.slider_ratio)
        if split_x > 0:
            left_crop = self.display_left.crop((0, 0, split_x, self.display_left.height))
            combined.paste(left_crop, (0, 0))

        self.tk_image = ImageTk.PhotoImage(combined)
        self.canvas.create_image(image_x, image_y, anchor=tk.NW, image=self.tk_image)

        slider_x = image_x + split_x
        self._draw_slider(slider_x, image_y, self.display_left.height)
        self._draw_labels(image_x, image_y, self.display_left.width)

    def _fit_image(
        self, image: Image.Image, canvas_width: int, canvas_height: int
    ) -> tuple[Image.Image, int, int]:
        max_width = max(canvas_width - 40, 1)
        max_height = max(canvas_height - 44, 1)
        scale = min(max_width / image.width, max_height / image.height)
        new_width = max(1, int(image.width * scale))
        new_height = max(1, int(image.height * scale))
        resampling_filter = getattr(Image, "Resampling", Image).LANCZOS
        resized = image.resize((new_width, new_height), resampling_filter)
        x_position = (canvas_width - new_width) // 2
        y_position = (canvas_height - new_height) // 2
        return resized, x_position, y_position

    def _draw_empty_state(self, canvas_width: int, canvas_height: int) -> None:
        self.canvas.create_text(
            canvas_width // 2,
            canvas_height // 2,
            text="Choose two images to compare",
            fill=MUTED_TEXT_COLOR,
            font=("Helvetica", 18, "bold"),
        )

    def _draw_slider(self, slider_x: int, image_y: int, image_height: int) -> None:
        self.canvas.create_line(
            slider_x,
            image_y,
            slider_x,
            image_y + image_height,
            fill=SLIDER_COLOR,
            width=3,
        )
        handle_radius = 18
        handle_y = image_y + image_height // 2
        self.canvas.create_oval(
            slider_x - handle_radius,
            handle_y - handle_radius,
            slider_x + handle_radius,
            handle_y + handle_radius,
            fill=ACCENT_COLOR,
            outline=SLIDER_COLOR,
            width=2,
        )
        self.canvas.create_text(
            slider_x,
            handle_y,
            text="< >",
            fill=TEXT_COLOR,
            font=("Helvetica", 9, "bold"),
        )

    def _draw_labels(self, image_x: int, image_y: int, image_width: int) -> None:
        label_y = image_y + 22
        self._draw_label(image_x + 20, label_y, self.left_name_var.get(), anchor=tk.W)
        self._draw_label(
            image_x + image_width - 20,
            label_y,
            self.right_name_var.get(),
            anchor=tk.E,
        )

    def _draw_label(self, x_position: int, y_position: int, text: str, anchor: str) -> None:
        label_text = text.strip() or "Untitled image"
        text_id = self.canvas.create_text(
            x_position,
            y_position,
            text=label_text,
            anchor=anchor,
            fill=TEXT_COLOR,
            font=("Helvetica", 12, "bold"),
        )
        bbox = self.canvas.bbox(text_id)
        if bbox is None:
            return

        padding_x = 10
        padding_y = 6
        rect_id = self.canvas.create_rectangle(
            bbox[0] - padding_x,
            bbox[1] - padding_y,
            bbox[2] + padding_x,
            bbox[3] + padding_y,
            fill="#000000",
            outline="",
            stipple="gray50",
        )
        self.canvas.tag_lower(rect_id, text_id)


def validate_path(path: Path | None, side: str) -> Path | None:
    if path is None:
        return None

    if not path.exists():
        raise SystemExit(f"{side} image does not exist: {path}")

    if not path.is_file():
        raise SystemExit(f"{side} image is not a file: {path}")

    return path


def main() -> int:
    args = parse_args()
    left_path = validate_path(args.left, "Left")
    right_path = validate_path(args.right, "Right")

    root = tk.Tk()
    ImageSliderViewer(
        root=root,
        left_path=left_path,
        right_path=right_path,
        left_name=args.left_name,
        right_name=args.right_name,
    )
    root.mainloop()
    return 0


if __name__ == "__main__":
    sys.exit(main())
