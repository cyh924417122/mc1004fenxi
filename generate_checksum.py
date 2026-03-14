#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

# 生成0-9的映射表（基于伪随机排序）
def generate_digit_permutation_map(input_seed, out_map):
    # 如果种子直接被模数整除，则用997避免全0序列
    if not (input_seed % 0x14DB3):
        input_seed = 997
    
    # 生成10个伪随机值（线性同余PRNG）
    rand_values = []
    for i in range(10):
        input_seed = 997 * input_seed % 0x14DB3
        rand_values.append(input_seed)
    
    # 初始化映射位置计数
    for i in range(10):
        out_map[i] = 0
    
    # 计算"排名"（0..9）: 值越小排名越小
    for i in range(10):
        for j in range(i + 1, 10):
            if rand_values[i] >= rand_values[j]:
                out_map[j] += 1
            else:
                out_map[i] += 1
    
    return input_seed

# 单字节滚动哈希
def digit_hash_step(byte_value, current_hash):
    result = current_hash
    while byte_value:
        digit = byte_value % 10
        byte_value //= 10
        result = 991 * (digit + result) % 997
    return result

# 计算数据哈希
def compute_data_hash(data_ptr, len):
    hash_val = 97  # 初始值 97
    for i in range(len):
        hash_val = digit_hash_step(data_ptr[i], hash_val)
    return hash_val

# 生成20位校验码
def generate_20_digit_checksum(product_id, registration_payload):
    # 解析注册载荷
    # 前12字节是产品ID的字符串表示
    src = registration_payload[:12]
    # 接下来3字节是OEM编码字符
    oem_chars = registration_payload[12:15]
    # 最后1字节是尾部字符
    tail_char = registration_payload[15]
    
    # 计算种子参数（使用前12字节作为种子）
    seed_param = [int(c) for c in src] + [0] * 4  # 补全16字节
    
    # 生成字段数据
    # 假设产品码、日期、尾数
    product_code = product_id
    # 简单起见，使用固定的日期和尾数
    date_code = 260512  # 假设日期
    tail_code = 20  # 假设尾数
    
    # 构建v22数组
    v22 = [0] * 40
    
    # 填充产品码（8位）- 注意：解码时v22[0]是最低位，v22[7]是最高位
    # 解码时的提取方式：v18 = 10*v22[1] + v22[0] + 100*v22[2] + 1000*v22[3] + 10000*v22[4] + 100000*v22[5] + 1000000*v22[6] + 10000000*v22[7]
    temp = product_code
    v22[0] = temp % 10
    temp //= 10
    v22[1] = temp % 10
    temp //= 10
    v22[2] = temp % 10
    temp //= 10
    v22[3] = temp % 10
    temp //= 10
    v22[4] = temp % 10
    temp //= 10
    v22[5] = temp % 10
    temp //= 10
    v22[6] = temp % 10
    temp //= 10
    v22[7] = temp % 10
    
    # 填充日期（4位）- 注意：解码时v22[8]是最低位，v22[11]是最高位
    # 解码时的提取方式：v21 = 10*v22[9] + v22[8] + 100*v22[10] + 1000*v22[11]
    temp = date_code
    v22[8] = temp % 10
    temp //= 10
    v22[9] = temp % 10
    temp //= 10
    v22[10] = temp % 10
    temp //= 10
    v22[11] = temp % 10
    
    # 填充尾数（4位）- 注意：解码时v22[12]是最低位，v22[15]是最高位
    # 解码时的提取方式：v20 = 10*v22[13] + v22[12] + 100*v22[14] + 1000*v22[15]
    temp = tail_code
    v22[12] = temp % 10
    temp //= 10
    v22[13] = temp % 10
    temp //= 10
    v22[14] = temp % 10
    temp //= 10
    v22[15] = temp % 10
    
    # 计算校验值
    v9 = compute_data_hash(v22, 16)
    
    # 填充校验值（3位）- 注意：解码时v22[18]是最高位，v22[16]是最低位
    temp = v9
    v22[16] = temp % 10
    temp //= 10
    v22[17] = temp % 10
    temp //= 10
    v22[18] = temp % 10
    
    # 计算校验和
    v10 = sum(v22[:19])
    v5 = v10 % 10  # 偏移检验值
    
    # 旋转数据
    # 旋转前先复制一份
    for i in range(19):
        v22[i + 19] = v22[i]
    
    # 执行旋转（与解码时相反的操作）
    # 解码时是 v22[k] = v22[k + offset]
    # 所以生成时应该是 v22[(k + offset) % 19] = v22[19 + k]
    offset = 3 + v5
    for k in range(19):
        v22[(k + offset) % 19] = v22[19 + k]
    
    # 计算种子哈希
    v17 = compute_data_hash(seed_param, 16)
    
    # 生成数字映射表
    v23 = [0] * 16
    generate_digit_permutation_map(v17 + v5, v23)
    
    # 正向映射：将序号映射为值
    input_data = [0] * 20
    for i in range(19):
        input_data[i] = v23[v22[i]]
    
    # 添加偏移检验值
    input_data[19] = v5
    
    # 转换为字符串
    checksum = ''.join(map(str, input_data))
    
    return checksum

# 测试函数
def test_generate_checksum():
    product_id = 15044657
    registration_payload = "231251190900KQRX"
    
    checksum = generate_20_digit_checksum(product_id, registration_payload)
    
    print(f"产品ID: {product_id}")
    print(f"注册载荷: {registration_payload}")
    print(f"生成的20位校验码: {checksum}")

if __name__ == "__main__":
    test_generate_checksum()
