# -*- coding: utf-8 -*-
import tkinter as tk
from tkinter import ttk

# 定义字符集
A_ABCDEFGHJKMNPQRSTUVWXYZ = "ABCDEFGHJKMNPQRSTUVWXYZ"  # 23个可用字符（无I与O）

# 计算产品ID的OEM哈希
def compute_oem_hash(product_id):
    hash_val = 97
    while product_id:
        digit = product_id % 23
        product_id //= 23
        hash_val = 12157 * (digit + hash_val) % 12167
    return hash_val

# 将哈希值编码为3个字母字符
def encode_oem_chars(oem_value):
    hash_val = compute_oem_hash(oem_value)
    chars = []
    # 计算三个字符
    chars.append(A_ABCDEFGHJKMNPQRSTUVWXYZ[hash_val % 23])
    hash_val //= 23
    chars.append(A_ABCDEFGHJKMNPQRSTUVWXYZ[hash_val % 23])
    high_part = hash_val // 23
    chars.append(A_ABCDEFGHJKMNPQRSTUVWXYZ[high_part % 23])
    # 反转顺序，因为计算时是从低位到高位
    return ''.join(reversed(chars))

# 处理计算按钮点击
def calculate():
    try:
        product_id = int(entry.get())
        oem_hash = compute_oem_hash(product_id)
        encoded_chars = encode_oem_chars(product_id)
        result_text.set(f"产品ID: {product_id}\nOEM哈希: {oem_hash}\n编码字符: {encoded_chars}")
    except ValueError:
        result_text.set("请输入有效的整数产品ID")

# 创建主窗口
root = tk.Tk()
root.title("OEM哈希计算工具")
root.geometry("400x200")

# 创建输入框和标签
frame = ttk.Frame(root, padding="10")
frame.pack(fill=tk.BOTH, expand=True)

label = ttk.Label(frame, text="产品ID:")
label.grid(row=0, column=0, sticky=tk.W, padx=5, pady=5)

entry = ttk.Entry(frame, width=20)
entry.grid(row=0, column=1, sticky=tk.EW, padx=5, pady=5)
entry.insert(0, "15044657")  # 默认值为15044657

# 创建计算按钮
calculate_btn = ttk.Button(frame, text="计算", command=calculate)
calculate_btn.grid(row=1, column=0, columnspan=2, pady=10)

# 创建结果显示
result_text = tk.StringVar()
result_label = ttk.Label(frame, textvariable=result_text, justify=tk.LEFT)
result_label.grid(row=2, column=0, columnspan=2, sticky=tk.W, padx=5, pady=5)

# 初始化计算
calculate()

# 启动主循环
root.mainloop()