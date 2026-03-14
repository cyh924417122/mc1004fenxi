#!/usr/bin/env python3

# 定义字符集
A_ABCDEFGHJKMNPQRSTUVWXYZ = "ABCDEFGHJKMNPQRSTUVWXYZ"  # 23个可用字符（无I与O）
A_ACDEFGHJKLMNPQRSTUVWXYZ = "ACDEFGHJKLMNPQRSTUVWXYZ"  # 23个可用字符（无B/I/O）

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

# 复制字节
def memcpy_bytes(src, dst, len):
    # 模拟C语言的memcpy函数
    # 这里我们只需要逻辑上的复制，实际在Python中不需要真正操作内存
    return src

# 组合注册载荷
def compose_registration_payload(dst, src, tail_code, oem_value):
    # 复制前12字节数据
    memcpy_bytes(src, dst, 12)
    # 编码OEM值为3个字符
    oem_chars = encode_oem_chars(oem_value)
    # 构建完整的注册载荷
    payload = src[:12]  # 前12字节
    payload += oem_chars  # 接下来3字节
    # 添加尾部字符
    tail_char = A_ACDEFGHJKLMNPQRSTUVWXYZ[tail_code % 23]
    payload += tail_char  # 最后1字节
    return payload

# 主函数
if __name__ == "__main__":
    # 测试ID为231251190900
    product_id = 15044657
    oem_hash = compute_oem_hash(product_id)
    encoded_chars = encode_oem_chars(product_id)
    
    # 组合注册载荷（假设src前12字节为"000000000000"，tail_code为10）
    src = "231251190900"  # 假设的前12字节数据
    tail_code = 20  # 假设的尾部代码
    payload = compose_registration_payload("", src, tail_code, product_id)
    
    print(f"产品ID: {product_id}")
    print(f"OEM哈希: {oem_hash}")
    print(f"编码字符: {encoded_chars}")
    print(f"注册载荷: {payload}")
