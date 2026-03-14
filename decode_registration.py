#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
    
    # 计算种子映射哈希（输入seed_param, 长度16）
    # 注意：这里我们假设seed_param是一个16字节的数组
    # 在实际使用中，需要根据具体情况调整
    v17 = compute_data_hash(seed_param, 16)
    
    # 生成数字映射表（顺序->值），结果保存在 v23[10]
    generate_digit_permutation_map(v17 + v5, v23)
    
    # 逆向映射：将前19字节数据值反推映射序号
    for i in range(19):
        for j in range(10):
            if input_data[i] == v23[j]:
                v22[i] = j
                v22[i + 19] = j
                break
    
    # 旋转，还原数据顺序；起始偏移 = 3 + v5
    for k in range(19):
        v22[k] = v22[k + 3 + v5]
    
    # 提取字段：产品码/日期/尾数/校验？
    v18 = (10 * v22[1] + v22[0] + 100 * v22[2] + 1000 * v22[3] +
           10000 * v22[4] + 100000 * v22[5] + 1000000 * v22[6] + 10000000 * v22[7])
    
    v21 = 10 * v22[9] + v22[8] + 100 * v22[10] + 1000 * v22[11]
    v20 = 10 * v22[13] + v22[12] + 100 * v22[14] + 1000 * v22[15]
    v19 = 10 * v22[17] + v22[16] + 100 * v22[18]
    
    # 计算重新编码哈希（以辨别数据一致性）
    v9 = compute_data_hash(v22, 16)
    
    # 校验 19 位和尾偏移
    v10 = 0
    for m in range(19):
        v10 += v22[m]
    
    if v10 % 10 == v5:
        if v9 == v19:
            v14 = v18
            v15 = v21
            v16 = v20
        else:
            v13 = 2  # 内部校验失败
        return copy_memory_optimized(output_buf, [v13, v14, v15, v16], 16)
    else:
        v13 = 1  # 校验和失败
        return copy_memory_optimized(output_buf, [v13, v14, v15, v16], 16)

# 测试函数
def test_decode_verify():
    # 测试数据
    # 假设输入注册码数据（20字节）
    input_data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 5]
    # 假设种子参数（16字节）
    seed_param = [1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6]
    # 输出缓冲区
    output_buf = [0] * 16
    
    # 调用解码和验证函数
    result = decode_and_verify_registration_payload(output_buf, input_data, seed_param)
    
    # 打印结果
    print("解码和验证结果:")
    print(f"状态码: {output_buf[0]}")
    print(f"产品码: {output_buf[1]}")
    print(f"日期: {output_buf[2]}")
    print(f"尾数: {output_buf[3]}")

# 验证之前生成的校验码
def test_verify_generated_checksum():
    # 使用之前生成的20位校验码
    checksum = "29592544820702887880"
    # 转换为整数列表
    input_data = [int(c) for c in checksum]
    
    # 种子参数：使用注册载荷的前12字节 "231251190900"
    seed_param = [2, 3, 1, 2, 5, 1, 1, 9, 0, 9, 0, 0] + [0] * 4  # 补全16字节
    
    # 输出缓冲区
    output_buf = [0] * 16
    
    # 调用解码和验证函数
    result = decode_and_verify_registration_payload(output_buf, input_data, seed_param)
    
    # 打印结果
    print("\n验证之前生成的校验码结果:")
    print(f"输入校验码: {checksum}")
    print(f"状态码: {output_buf[0]}")
    print(f"产品码: {output_buf[1]}")
    print(f"日期: {output_buf[2]}")
    print(f"尾数: {output_buf[3]}")
    
    # 验证状态码
    if output_buf[0] == 0:
        print("\n校验码验证成功！")
    else:
        print(f"\n校验码验证失败，错误码: {output_buf[0]}")

if __name__ == "__main__":
    test_decode_verify()
    test_verify_generated_checksum()
