import sys
import random
from typing import List
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QTextEdit, QLineEdit, QLabel, QFileDialog, QFrame
)
from PyQt6.QtGui import QFont, QColor, QPalette
from PyQt6.QtCore import Qt

class BitConverter:
    @staticmethod
    def string_to_bits(input_text: str, encoding='utf-16-le') -> List[bool]:
        if not input_text:
            return []
        data = input_text.encode(encoding, errors='ignore')
        bits = []
        for b in data:
            for i in range(8):
                bits.append((b & (1 << (7 - i))) != 0)
        return bits

    @staticmethod
    def bits_to_string(bits: List[bool], encoding='utf-16-le') -> str:
        if not bits:
            return ''
        byte_count = len(bits) // 8
        b = bytearray(byte_count)
        for i in range(byte_count):
            val = 0
            for j in range(8):
                if bits[i * 8 + j]:
                    val |= (1 << (7 - j))
            b[i] = val
        try:
            return b.decode(encoding, errors='ignore')
        except Exception:
            return b.decode('utf-8', errors='ignore')

    @staticmethod
    def bytes_to_hex(bytes_arr: bytes) -> str:
        return " ".join(f"{x:02X}" for x in bytes_arr)

    @staticmethod
    def hex_to_bytes(hex_str: str) -> bytes:
        if not hex_str:
            return b''
        clean = hex_str.replace(" ", "")
        if len(clean) % 2 != 0:
            raise ValueError("HEX string must contain even number of characters")
        b = bytes(int(clean[i:i+2], 16) for i in range(0, len(clean), 2))
        return b

    @staticmethod
    def bits_to_hex(bits: List[bool]) -> str:
        bytes_count = (len(bits) + 7) // 8
        arr = bytearray(bytes_count)
        for i, bit in enumerate(bits):
            if bit:
                byte_index = i // 8
                bit_index = 7 - (i % 8)
                arr[byte_index] |= (1 << bit_index)
        return BitConverter.bytes_to_hex(bytes(arr))

    @staticmethod
    def hex_to_bits(hex_str: str) -> List[bool]:
        b = BitConverter.hex_to_bytes(hex_str)
        bits = []
        for byte in b:
            for i in range(8):
                bits.append((byte & (1 << (7 - i))) != 0)
        return bits

    @staticmethod
    def bytes_to_bits(bytes_arr: bytes) -> List[bool]:
        bits = []
        for byte in bytes_arr:
            for i in range(8):
                bits.append((byte & (1 << (7 - i))) != 0)
        return bits

    @staticmethod
    def bits_to_bytes(bits: List[bool]) -> bytes:
        if not bits:
            return b''
        byte_count = (len(bits) + 7) // 8
        arr = bytearray(byte_count)
        for i in range(len(bits)):
            if bits[i]:
                byte_index = i // 8
                bit_index = 7 - (i % 8)
                arr[byte_index] |= (1 << bit_index)
        return bytes(arr)

    @staticmethod
    def int_to_bits(value: int, length: int) -> List[bool]:
        bits = [False] * length
        for i in range(length):
            bits[length - 1 - i] = ((value >> i) & 1) == 1
        return bits

class DESKeyGenerator:
    @staticmethod
    def validate_key_parity(key64bit: List[bool]) -> bool:
        if len(key64bit) != 64:
            return False
        for i in range(8):
            parity_index = i * 8 + 7
            ones = 0
            for j in range(7):
                if key64bit[i * 8 + j]:
                    ones += 1
            expected_parity = (ones % 2) == 1
            if key64bit[parity_index] != expected_parity:
                return False
        return True

    @staticmethod
    def add_parity_bits(key56: List[bool]) -> List[bool]:
        if len(key56) != 56:
            raise ValueError("56-bit key required")
        key64 = [False] * 64
        for i in range(8):
            for j in range(7):
                key64[i * 8 + j] = key56[i * 7 + j]
            ones = sum(1 for j in range(7) if key56[i * 7 + j])
            key64[i * 8 + 7] = (ones % 2) == 1
        return key64

    @staticmethod
    def generate_random_64bit_key_hex() -> str:
        key_bytes = bytes(random.getrandbits(8) for _ in range(7))
        key56_bits = BitConverter.bytes_to_bits(key_bytes)
        key64_bits = DESKeyGenerator.add_parity_bits(key56_bits)
        return BitConverter.bits_to_hex(key64_bits)

class DESEncryption:
    IP = [58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7]
    FP = [40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25]
    E = [32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,12,13,12,13,14,15,16,17,16,17,18,19,20,21,20,21,22,23,24,25,24,25,26,27,28,29,28,29,30,31,32,1]
    P = [16,7,20,21,29,12,28,17,1,15,23,26,5,18,31,10,2,8,24,14,32,27,3,9,19,13,30,6,22,11,4,25]
    S_BOXES = [
        [[14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7],[0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8],[4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0],[15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13]],
        [[15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10],[3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5],[0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15],[13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9]],
        [[10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8],[13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1],[13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7],[1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12]],
        [[7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15],[13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9],[10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4],[3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14]],
        [[2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9],[14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6],[4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14],[11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3]],
        [[12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11],[10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8],[9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6],[4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13]],
        [[4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1],[13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6],[1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2],[6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12]],
        [[13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7],[1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2],[7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8],[2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11]]
    ]
    PC1 = [57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,35,27,19,11,3,60,52,44,36,63,55,47,39,31,23,15,7,62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,28,20,12,4]
    PC2 = [14,17,11,24,1,5,3,28,15,6,21,10,23,19,12,4,26,8,16,7,27,20,13,2,41,52,31,37,47,55,30,40,51,45,33,48,44,49,39,56,34,53,46,42,50,36,29,32]
    SHIFTS = [1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1]

    @staticmethod
    def permute(input_bits: List[bool], table: List[int]) -> List[bool]:
        return [input_bits[i - 1] for i in table]

    @staticmethod
    def xor(a: List[bool], b: List[bool]) -> List[bool]:
        return [x ^ y for x, y in zip(a, b)]

    @staticmethod
    def left_shift(bits: List[bool], shifts: int) -> List[bool]:
        return bits[shifts:] + bits[:shifts]

    @staticmethod
    def generate_subkeys(key64: List[bool]) -> List[List[bool]]:
        perm = DESEncryption.permute(key64, DESEncryption.PC1)
        left = perm[:28]
        right = perm[28:]
        subkeys = []
        for i in range(16):
            left = DESEncryption.left_shift(left, DESEncryption.SHIFTS[i])
            right = DESEncryption.left_shift(right, DESEncryption.SHIFTS[i])
            combined = left + right
            subkeys.append(DESEncryption.permute(combined, DESEncryption.PC2))
        return subkeys

    @staticmethod
    def f_func(right: List[bool], subkey: List[bool]) -> List[bool]:
        expanded = DESEncryption.permute(right, DESEncryption.E)
        xored = DESEncryption.xor(expanded, subkey)
        substituted = []
        for i in range(8):
            chunk = xored[i*6:(i+1)*6]
            row = (1 if chunk[0] else 0) * 2 + (1 if chunk[5] else 0)
            col = (1 if chunk[1] else 0) * 8 + (1 if chunk[2] else 0) * 4 + (1 if chunk[3] else 0) * 2 + (1 if chunk[4] else 0)
            val = DESEncryption.S_BOXES[i][row][col]
            bits4 = BitConverter.int_to_bits(val, 4)
            substituted.extend(bits4)
        return DESEncryption.permute(substituted, DESEncryption.P)

    @staticmethod
    def encrypt_block64(in_text_block: str, key_hex: str) -> str:
        key_bits = BitConverter.hex_to_bits(key_hex)
        input_bits = BitConverter.string_to_bits(in_text_block)
        if len(input_bits) != 64 or len(key_bits) != 64:
            return ""
        if not DESKeyGenerator.validate_key_parity(key_bits):
            return ""
        subkeys = DESEncryption.generate_subkeys(key_bits)
        permuted = DESEncryption.permute(input_bits, DESEncryption.IP)
        left = permuted[:32]
        right = permuted[32:]
        for i in range(16):
            temp = right[:]
            right = DESEncryption.xor(left, DESEncryption.f_func(right, subkeys[i]))
            left = temp
        combined = right + left
        output_bits = DESEncryption.permute(combined, DESEncryption.FP)
        return BitConverter.bits_to_hex(output_bits) + " "

    @staticmethod
    def decrypt_block64(encrypted_hex_block: str, key_hex: str) -> str:
        enc_bits = BitConverter.hex_to_bits(encrypted_hex_block)
        key_bits = BitConverter.hex_to_bits(key_hex)
        if len(enc_bits) != 64 or len(key_bits) != 64:
            return ""
        if not DESKeyGenerator.validate_key_parity(key_bits):
            raise ValueError("Invalid key parity")
        subkeys = DESEncryption.generate_subkeys(key_bits)
        permuted = DESEncryption.permute(enc_bits, DESEncryption.IP)
        left = permuted[:32]
        right = permuted[32:]
        for i in range(15, -1, -1):
            temp = left[:]
            left = DESEncryption.xor(right, DESEncryption.f_func(left, subkeys[i]))
            right = temp
        combined = left + right
        output_bits = DESEncryption.permute(combined, DESEncryption.FP)
        return BitConverter.bits_to_string(output_bits)

class CryptoApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DES Cryptography — UTF-16 Support")
        self.resize(1100, 650)
        self.init_ui()

    def init_ui(self):
        palette = QPalette()
        palette.setColor(QPalette.ColorRole.Window, QColor("#1E1E2E"))
        palette.setColor(QPalette.ColorRole.Base, QColor("#2E2E3E"))
        palette.setColor(QPalette.ColorRole.Text, Qt.GlobalColor.white)
        self.setPalette(palette)
        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(12)
        key_layout = QHBoxLayout()
        key_label = QLabel("DES Key:")
        key_label.setStyleSheet("color: white; font-size: 16px;")
        self.key_field = QLineEdit()
        self.key_field.setPlaceholderText("Example: BB B1 F3 D2 9C 8D 88")
        self.key_field.setFont(QFont("Consolas", 12))
        self.key_field.setStyleSheet("background-color:#2E2E3E; color:white; border:1px solid #555; padding:6px;")
        gen_btn = QPushButton("Generate")
        gen_btn.setStyleSheet("background-color:#7B5AF2; color:white; font-weight:bold; padding:8px 14px; border-radius:6px;")
        gen_btn.clicked.connect(self.on_generate_key)
        key_imp_btn = QPushButton("Import Key")
        key_exp_btn = QPushButton("Export Key")
        key_imp_btn.setStyleSheet("background:#6E6EDC; color:white; padding:8px 10px; border-radius:6px;")
        key_exp_btn.setStyleSheet("background:#6E6EDC; color:white; padding:8px 10px; border-radius:6px;")
        key_imp_btn.clicked.connect(lambda: self.import_text(self.key_field))
        key_exp_btn.clicked.connect(lambda: self.export_text(self.key_field))
        key_layout.addWidget(key_label)
        key_layout.addWidget(self.key_field, 3)
        key_layout.addWidget(gen_btn)
        key_layout.addWidget(key_imp_btn)
        key_layout.addWidget(key_exp_btn)
        content_layout = QHBoxLayout()
        content_layout.setSpacing(18)
        left_v = QVBoxLayout()
        left_label = QLabel("Plain Text:")
        left_label.setStyleSheet("color: #A0A0A0; font-size: 15px;")
        self.input_text = QTextEdit()
        self.input_text.setStyleSheet("background-color:#2E2E3E; color:white; border:none; padding:8px; border-radius:8px;")
        self.input_text.setFont(QFont("Consolas", 12))
        left_buttons = QHBoxLayout()
        imp_left = QPushButton("Import Plain")
        exp_left = QPushButton("Export Plain")
        imp_left.setStyleSheet("background:#7B5AF2; color:white; padding:6px 10px; border-radius:6px;")
        exp_left.setStyleSheet("background:#7B5AF2; color:white; padding:6px 10px; border-radius:6px;")
        imp_left.clicked.connect(lambda: self.import_text(self.input_text))
        exp_left.clicked.connect(lambda: self.export_text(self.input_text))
        left_buttons.addWidget(imp_left)
        left_buttons.addWidget(exp_left)
        left_v.addWidget(left_label)
        left_v.addWidget(self.input_text)
        left_v.addLayout(left_buttons)
        divider = QFrame()
        divider.setFrameShape(QFrame.Shape.VLine)
        divider.setFrameShadow(QFrame.Shadow.Sunken)
        divider.setStyleSheet("color:#3E3E4E;")
        right_v = QVBoxLayout()
        right_label = QLabel("Encrypted / Decrypted Text:")
        right_label.setStyleSheet("color: #A0A0A0; font-size: 15px;")
        self.output_text = QTextEdit()
        self.output_text.setStyleSheet("background-color:#2E2E3E; color:white; border:none; padding:8px; border-radius:8px;")
        self.output_text.setFont(QFont("Consolas", 12))
        right_buttons = QHBoxLayout()
        imp_right = QPushButton("Import Enc")
        exp_right = QPushButton("Export Enc")
        imp_right.setStyleSheet("background:#7B5AF2; color:white; padding:6px 10px; border-radius:6px;")
        exp_right.setStyleSheet("background:#7B5AF2; color:white; padding:6px 10px; border-radius:6px;")
        imp_right.clicked.connect(lambda: self.import_text(self.output_text))
        exp_right.clicked.connect(lambda: self.export_text(self.output_text))
        right_buttons.addWidget(imp_right)
        right_buttons.addWidget(exp_right)
        right_v.addWidget(right_label)
        right_v.addWidget(self.output_text)
        right_v.addLayout(right_buttons)
        content_layout.addLayout(left_v, 1)
        content_layout.addWidget(divider)
        content_layout.addLayout(right_v, 1)
        bottom_layout = QHBoxLayout()
        bottom_layout.setSpacing(25)
        bottom_layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        encrypt_btn = QPushButton("ENCRYPT")
        decrypt_btn = QPushButton("DECRYPT")
        encrypt_btn.setStyleSheet("""
            QPushButton {
                background-color:#4ADE80;
                color:#000;
                font-weight:bold;
                font-size:18px;
                padding:12px 40px;
                border-radius:10px;
            }
            QPushButton:hover { background-color:#34D399; }
        """)
        decrypt_btn.setStyleSheet("""
            QPushButton {
                background-color:#F87171;
                color:white;
                font-weight:bold;
                font-size:18px;
                padding:12px 40px;
                border-radius:10px;
            }
            QPushButton:hover { background-color:#EF4444; }
        """)
        encrypt_btn.clicked.connect(self.on_encrypt)
        decrypt_btn.clicked.connect(self.on_decrypt)
        bottom_layout.addWidget(encrypt_btn)
        bottom_layout.addWidget(decrypt_btn)
        main_layout.addLayout(key_layout)
        main_layout.addLayout(content_layout)
        main_layout.addLayout(bottom_layout)

    def on_generate_key(self):
        self.key_field.setText(DESKeyGenerator.generate_random_64bit_key_hex())

    def on_encrypt(self):
        text = self.input_text.toPlainText()
        key_hex = self.key_field.text().strip()
        if not text or not key_hex:
            return
        block_count = (len(text) + 3) // 4
        pad = block_count * 4 - len(text)
        text_padded = text + ("\0" * pad)
        result = []
        for i in range(block_count):
            blk = text_padded[i*4:(i+1)*4]
            enc = DESEncryption.encrypt_block64(blk, key_hex)
            if not enc:
                self.output_text.setPlainText("Ошибка: неверный ключ или блок")
                return
            result.append(enc)
        self.output_text.setPlainText("".join(result).strip())

    def on_decrypt(self):
        enc_text = self.output_text.toPlainText().strip()
        key_hex = self.key_field.text().strip()
        if not enc_text or not key_hex:
            return
        tokens = enc_text.split()
        if len(tokens) % 8 != 0:
            joined = "".join(tokens)
            if len(joined) % 16 != 0:
                self.input_text.setPlainText("Неверный формат зашифрованного текста")
                return
            blocks = [joined[i:i+16] for i in range(0, len(joined), 16)]
            blocks_hex = [" ".join(blocks[i][j:j+2] for j in range(0, 16, 2)) for i in range(len(blocks))]
        else:
            blocks_hex = []
            for i in range(0, len(tokens), 8):
                blk = tokens[i:i+8]
                blocks_hex.append(" ".join(blk))
        out = []
        for blk in blocks_hex:
            try:
                dec = DESEncryption.decrypt_block64(blk, key_hex)
            except Exception as ex:
                self.input_text.setPlainText(f"Ошибка дешифрования: {ex}")
                return
            if not dec:
                self.input_text.setPlainText("Ошибка дешифрования")
                return
            out.append(dec)
        final = "".join(out).rstrip('\0')
        self.input_text.setPlainText(final)

    def import_text(self, widget):
        path, _ = QFileDialog.getOpenFileName(self, "Open file", "", "Text Files (*.txt);;All Files (*.*)")
        if path:
            with open(path, "r", encoding="utf-8", errors="ignore") as f:
                data = f.read()
            if isinstance(widget, QTextEdit):
                widget.setPlainText(data)
            elif isinstance(widget, QLineEdit):
                widget.setText(data)

    def export_text(self, widget):
        path, _ = QFileDialog.getSaveFileName(self, "Save file", "", "Text Files (*.txt);;All Files (*.*)")
        if path:
            data = widget.toPlainText() if isinstance(widget, QTextEdit) else widget.text()
            with open(path, "w", encoding="utf-8", errors="ignore") as f:
                f.write(data)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    w = CryptoApp()
    w.show()
    sys.exit(app.exec())
