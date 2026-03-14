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

# 复制内存（模拟C语言的memcpy）
def copy_memory_optimized(result, a2, a3):
    # 复制数据到输出缓冲区
    for i in range(min(len(a2), len(result))):
        result[i] = a2[i]
    return result

# 解码并验证注册载荷
def decode_and_verify_registration_payload(output_buf, input_data, seed_param):
    # 初始化变量
    v13 = 0
    v14 = 0
    v15 = 0
    v16 = 0
    v22 = [0] * 40  # 40字节缓冲区
    v23 = [0] * 16  # 16字节映射表
    
    # 读取偏移检验值（最后一位）
    v5 = input_data[19]
    print(f"v5 (偏移检验值): {v5}")
    
    # 计算种子映射哈希（输入seed_param, 长度16）
    v17 = compute_data_hash(seed_param, 16)
    print(f"v17 (种子哈希): {v17}")
    
    # 生成数字映射表（顺序->值），结果保存在 v23[10]
    generate_digit_permutation_map(v17 + v5, v23)
    print(f"v23 (映射表): {v23[:10]}")
    
    # 逆向映射：将前19字节数据值反推映射序号
    for i in range(19):
        for j in range(10):
            if input_data[i] == v23[j]:
                v22[i] = j
                v22[i + 19] = j
                break
    print(f"v22 (逆向映射后): {v22[:19]}")
    
    # 旋转，还原数据顺序；起始偏移 = 3 + v5
    offset = 3 + v5
    print(f"offset (旋转偏移): {offset}")
    for k in range(19):
        v22[k] = v22[k + offset]
    print(f"v22 (旋转后): {v22[:19]}")
    
    # 提取字段：产品码/日期/尾数/校验？
    v18 = (10 * v22[1] + v22[0] + 100 * v22[2] + 1000 * v22[3] +
           10000 * v22[4] + 100000 * v22[5] + 1000000 * v22[6] + 10000000 * v22[7])
    
    v21 = 10 * v22[9] + v22[8] + 100 * v22[10] + 1000 * v22[11]
    v20 = 10 * v22[13] + v22[12] + 100 * v22[14] + 1000 * v22[15]
    v19 = 10 * v22[17] + v22[16] + 100 * v22[18]
    
    print(f"v18 (产品码): {v18}")
    print(f"v21 (日期): {v21}")
    print(f"v20 (尾数): {v20}")
    print(f"v19 (校验值): {v19}")
    
    # 计算重新编码哈希（以辨别数据一致性）
    v9 = compute_data_hash(v22, 16)
    print(f"v9 (计算的哈希): {v9}")
    
    # 校验 19 位和尾偏移
    v10 = 0
    for m in range(19):
        v10 += v22[m]
    print(f"v10 (校验和): {v10}")
    print(f"v10 % 10 == v5: {v10 % 10 == v5}")
    print(f"v9 == v19: {v9 == v19}")
    
    if v10 % 10 == v5:
        if v9 == v19:
            v14 = v18
            v15 = v21
            v16 = v20
            print("校验成功！")
        else:
            v13 = 2  # 内部校验失败
            print("内部校验失败！")
        return copy_memory_optimized(output_buf, [v13, v14, v15, v16], 16)
    else:
        v13 = 1  # 校验和失败
        print("校验和失败！")
        return copy_memory_optimized(output_buf, [v13, v14, v15, v16], 16)

# 生成20位校验码
def generate_20_digit_checksum(product_id, registration_payload):
    # 解析注册载荷
    # 前12字节是产品ID的字符串表示
    src = registration_payload[:12]
    # 接下来3字节是OEM编码字符
    oem_chars = registration_payload[12:15]
    # 最后1字节是尾部字符
    tail_char = registration_payload[15]
    
    print(f"src: {src}")
    print(f"oem_chars: {oem_chars}")
    print(f"tail_char: {tail_char}")
    
    # 计算种子参数（使用前12字节作为种子）
    seed_param = [int(c) for c in src] + [0] * 4  # 补全16字节
    print(f"seed_param: {seed_param}")
    
    # 生成字段数据
    # 假设产品码、日期、尾数
    product_code = product_id
    # 简单起见，使用固定的日期和尾数
    date_code = 202312  # 假设日期
    tail_code = 123  # 假设尾数
    
    print(f"product_code: {product_code}")
    print(f"date_code: {date_code}")
    print(f"tail_code: {tail_code}")
    
    # 构建v22数组
    v22 = [0] * 40
    
    # 填充产品码（8位）- 注意：解码时v22[0]是最低位，v22[7]是最高位
    temp = product_code
    for i in range(8):
        v22[i] = temp % 10
        temp //= 10
    
    # 填充日期（4位）- 注意：解码时v22[8]是最低位，v22[11]是最高位
    temp = date_code
    for i in range(8, 12):
        v22[i] = temp % 10
        temp //= 10
    
    # 填充尾数（4位）- 注意：解码时v22[12]是最低位，v22[15]是最高位
    temp = tail_code
    for i in range(12, 16):
        v22[i] = temp % 10
        temp //= 10
    
    print(f"v22 (填充后): {v22[:19]}")
    
    # 计算校验值
    v9 = compute_data_hash(v22, 16)
    print(f"v9 (计算的哈希): {v9}")
    
    # 填充校验值（3位）- 注意：解码时v22[18]是最高位，v22[16]是最低位
    temp = v9
    v22[16] = temp % 10
    temp //= 10
    v22[17] = temp % 10
    temp //= 10
    v22[18] = temp % 10
    
    print(f"v22 (填充校验值后): {v22[:19]}")
    
    # 计算校验和
    v10 = sum(v22[:19])
    v5 = v10 % 10  # 偏移检验值
    print(f"v10 (校验和): {v10}")
    print(f"v5 (偏移检验值): {v5}")
    
    # 旋转数据
    # 旋转前先复制一份
    for i in range(19):
        v22[i + 19] = v22[i]
    
    print(f"v22 (复制后): {v22[:38]}")
    
    # 执行旋转（与解码时相反的操作）
    # 解码时是 v22[k] = v22[k + offset]
    # 所以生成时应该是 v22[k + offset] = v22[k]
    # 但需要从后向前处理，避免覆盖还未处理的数据
    offset = 3 + v5
    print(f"offset (旋转偏移): {offset}")
    for k in range(18, -1, -1):
        v22[k + offset] = v22[19 + k]
    
    print(f"v22 (旋转后): {v22[:19]}")
    
    # 计算种子哈希
    v17 = compute_data_hash(seed_param, 16)
    print(f"v17 (种子哈希): {v17}")
    
    # 生成数字映射表
    v23 = [0] * 16
    generate_digit_permutation_map(v17 + v5, v23)
    print(f"v23 (映射表): {v23[:10]}")
    
    # 正向映射：将序号映射为值
    input_data = [0] * 20
    for i in range(19):
        input_data[i] = v23[v22[i]]
    
    # 添加偏移检验值
    input_data[19] = v5
    
    print(f"input_data (生成的校验码): {input_data}")
    
    # 转换为字符串
    checksum = ''.join(map(str, input_data))
    
    return checksum

# 测试函数
def test_debug():
    product_id = 15044657
    registration_payload = "231251190900KQRX"
    
    print("===== 生成校验码 =====")
    checksum = generate_20_digit_checksum(product_id, registration_payload)
    print(f"生成的20位校验码: {checksum}")
    
    print("\n===== 验证校验码 =====")
    # 转换为整数列表
    input_data = [int(c) for c in checksum]
    
    # 种子参数：使用注册载荷的前12字节 "231251190900"
    seed_param = [2, 3, 1, 2, 5, 1, 1, 9, 0, 9, 0, 0] + [0] * 4  # 补全16字节
    
    # 输出缓冲区
    output_buf = [0] * 16
    
    # 调用解码和验证函数
    result = decode_and_verify_registration_payload(output_buf, input_data, seed_param)
    
    # 打印结果
    print("\n验证结果:")
    print(f"状态码: {output_buf[0]}")
    print(f"产品码: {output_buf[1]}")
    print(f"日期: {output_buf[2]}")
    print(f"尾数: {output_buf[3]}")

if __name__ == "__main__":
    test_debug()
