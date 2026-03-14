// 本文由D:\Documents\IDA\MC1004A\2312511900973MC1400M.c生成，未经人工校对，可能存在错误，仅供参考。
char aAbcdefghjkmnpq[24] = "ABCDEFGHJKMNPQRSTUVWXYZ"; // 23个可用字符（无I与O）
char aAcdefghjklmnpq[24] = "ACDEFGHJKLMNPQRSTUVWXYZ"; // 23个可用字符（无B/I/O）

//----- (0803D9B2) --------------------------------------------------------
// generate_digit_permutation_map
// 生成0-9的映射表（基于伪随机排序），对应 Python 中 _build_digit_map
// input_seed: 基于序列号的哈希种子值
// out_map: 10字节输出缓冲区，结果为0..9的一一映射
// return: 最终伪随机状态，用于连锁调用或调试
unsigned int __fastcall generate_digit_permutation_map(unsigned int input_seed, int out_map)
{
  unsigned int idx; // r3
  unsigned int idx2; // r3
  unsigned int idx3; // r3
  unsigned int inner; // r2
  _DWORD rand_values[10]; // [sp+0h] [bp-28h]

  // 如果种子直接被模数整除，则用997避免全0序列
  if ( !(input_seed % 0x14DB3) )
    input_seed = 997;

  // 生成10个伪随机值（线性同余PRNG）
  for ( idx = 0; idx < 10; ++idx )
  {
    input_seed = 997 * input_seed % 0x14DB3;
    rand_values[idx] = input_seed;
  }

  // 初始化映射位置计数
  for ( idx2 = 0; idx2 < 10; ++idx2 )
    *(_BYTE *)(idx2 + out_map) = 0;

  // 计算“排名”（0..9）: 值越小排名越小
  for ( idx3 = 0; idx3 < 10; ++idx3 )
  {
    for ( inner = idx3 + 1; inner < 10; ++inner )
    {
      if ( rand_values[idx3] >= rand_values[inner] )
        ++*(_BYTE *)(inner + out_map);
      else
        ++*(_BYTE *)(idx3 + out_map);
    }
  }

  return input_seed;
}

//----- (0803DA38) --------------------------------------------------------
// compute_oem_hash (sub_532658 等效)
// product_id -> OEM 23进制哈希值
// 取23进制每位，然后用 12157*(hash+digit)%12167 迭代
unsigned int __fastcall compute_oem_hash(unsigned int product_id)
{
  unsigned int hash; // r0
  unsigned int digit; // r2

  hash = 97;
  while ( product_id )
  {
    digit = product_id % 23;
    product_id /= 23;
    hash = 12157 * (digit + hash) % 12167;
  }
  return hash;
}



//----- (0803E4D6) --------------------------------------------------------
// copy_serial_and_set_oem_char
// a1: 目标缓存（至少16字节）
// a2: 源数据（至少12字节）
// a3: OEM索引参数（用于选字符）
// a4: 数据值透传（返回值）
unsigned int __fastcall copy_serial_and_set_oem_char(int dst, int src, unsigned __int8 oem_index, unsigned int passthrough)
{
  sub_8019DE4(src, dst, 0xCu);            // 复制前12字节
  sub_803E486(passthrough, (_BYTE *)(dst + 12));
  *(_BYTE *)(dst + 15) = aAcdefghjklmnpq[oem_index % 23];
  return passthrough;
}

//----- (08037712) --------------------------------------------------------
// process_machine_code_record
// 入口：a1=输出/写入目标，a2=输入参数id，a3,a4=额外上下文参数
// 功能：从全局资源加载机器码（sub_8014E04），计算 hash，写入结果，释放资源
// 返回 1=成功, 0=失败
int __fastcall process_machine_code_record(int output_buf, int input_id, int param3, int param4)
{
  unsigned __int8 *data_ptr; // r0
  unsigned __int8 *data_ptr_copy; // r6
  unsigned int hash_val; // r0
  unsigned __int8 status; // r5
  int resource_handle; // r0
  int t1; // r1
  int t2; // r2
  int t3; // r3

  data_ptr = (unsigned __int8 *)sub_8014E04((int)dword_2000CFD8, input_id, param3, param4);//31 35 30 34 34 36 35 37 00 00 00 00 00 00 00 00
  data_ptr_copy = data_ptr;
  if ( data_ptr )
  {
    hash_val = parse_number_string(data_ptr, 8u);//00E59031,调试跟踪为该数值的十六进制形式，解析后为15044657   149000017
    compose_registration_payload(output_buf, dword_20000A8C, *((_DWORD *)data_ptr_copy + 4), hash_val);  // dword_20000A8C 产品ID
    status = 1;
  }
  else
  {
    status = 0;
  }
  resource_handle = sub_8014DD0((int)dword_2000CFD8);
  sub_8026228(resource_handle, t1, t2, t3);
  return status;
}
//----- (08019C70) --------------------------------------------------------
// parse_number_string
// 将输入字符串解析为整型数值
// 支持两种格式：
//   - 0x/0X 前缀的十六进制
//   - 纯十进制（长度<9 或 首字符<'3'）
// 长度>=9 且不满足前条件时，返回错误值 300000000
int __fastcall parse_number_string(unsigned __int8 *str, unsigned __int8 len)
{
  int value; // r6
  unsigned __int8 i; // r3
  unsigned __int8 ch; // r5
  unsigned __int8 digit; // r4
  unsigned __int8 j; // r3

  value = 0;
  if ( *str == '0' && (str[1] == 'x' || str[1] == 'X') )
  {
    for ( i = 2; i < (unsigned int)len; ++i )
    {
      ch = str[i];
      if ( !ch || value >> 24 )
        break;
      if ( ch >= '0' && ch <= '9' )
        digit = ch - '0';
      else if ( ch >= 'A' && ch <= 'F' )
        digit = ch - 'A' + 10;
      else if ( ch >= 'a' && ch <= 'f' )
        digit = ch - 'a' + 10;
      else
        break;
      value = (16 * value) | digit;
    }
    return value;
  }
  else if ( *str < '3' || len < 9u )
  {
    for ( j = 0; j < (unsigned int)len; ++j )
      value = (str[j] - '0') + 10 * value;
    return value;
  }
  else
  {
    return 300000000;
  }
}

//----- (0803DA6E) --------------------------------------------------------
// encode_oem_chars
// 将 OEM 哈希值按 23 进制拆分为 3 个用户可读字母字符
// 输出: out_chars[0..2]
unsigned int __fastcall encode_oem_chars(unsigned int oem_value, _BYTE *out_chars)
{
  unsigned int hash_val; // r0
  unsigned int high_part; // r0

  hash_val = compute_oem_hash(oem_value);
  out_chars[2] = aAbcdefghjkmnpq[hash_val % 23];
  hash_val /= 23;
  out_chars[1] = aAbcdefghjkmnpq[hash_val % 23];
  high_part = hash_val / 23;
  out_chars[0] = aAbcdefghjkmnpq[high_part % 23];
  return high_part;
}

//----- (08019908) --------------------------------------------------------
// memcpy_bytes
// 从 src 复制 len 字节到 dst
int __fastcall memcpy_bytes(int src, int dst, unsigned __int16 len)
{
  unsigned __int16 i; // r3

  for ( i = 0; i < len; ++i )
    *(_BYTE *)(i + dst) = *(_BYTE *)(i + src);
  return src;
}
//----- (0803DABE) --------------------------------------------------------
// compose_registration_payload
// 复制前12字节数据，并写入 OEM 编码字符 + 后缀字节
unsigned int __fastcall compose_registration_payload(int dst, int src, unsigned __int8 tail_code, unsigned int oem_value)
{
  memcpy_bytes(src, dst, 0xCu);
  encode_oem_chars(oem_value, (_BYTE *)(dst + 12));
  *(_BYTE *)(dst + 15) = aAcdefghjklmnpq[tail_code % 23];
  return oem_value;
}

//----- (0803DAFA) --------------------------------------------------------
// decode_and_verify_registration_payload
// a1: 输出缓存指针（16字节）
// a2: 输入序列/注册码数据指针（至少20字节）
// a3: 种子参数（通常是序列号或产品ID相关值）
// 流程：
// 1. 读取第19位偏移检查位 v5
// 2. 计算种子哈希 v17 = compute_data_hash(a3, 16)
// 3. 生成映射表：generate_digit_permutation_map(v17+v5, v23)
// 4. 逆映射前19字节到v22，并复制一份便于旋转
// 5. 旋转 v22：起始偏移 = 3 + v5
// 6. 从旋转后v22提取4段字段：v18/v21/v20/v19
// 7. 计算校验哈希 v9 = compute_data_hash(v22, 16)
// 8. 计算和校验 sum(v22[0..18]) % 10 == v5
// 9. 2重校验通过则写padded输出并返回结果
_DWORD *__fastcall decode_and_verify_registration_payload(_DWORD *a1, int a2, int a3)
{
  unsigned __int8 v5; // r7
  int i; // r8
  int j; // r9
  int k; // r8
  unsigned int v9; // r10
  unsigned int v10; // r11
  int m; // r8
  int v13; // [sp+0h] [bp-5Ch] BYREF
  int v14; // [sp+4h] [bp-58h]
  int v15; // [sp+8h] [bp-54h]
  int v16; // [sp+Ch] [bp-50h]
  unsigned int v17; // [sp+10h] [bp-4Ch]
  int v18; // [sp+14h] [bp-48h]
  int v19; // [sp+18h] [bp-44h]
  int v20; // [sp+1Ch] [bp-40h]
  int v21; // [sp+20h] [bp-3Ch]
  _BYTE v22[40]; // [sp+24h] [bp-38h] BYREF
  _BYTE v23[16]; // [sp+4Ch] [bp-10h] BYREF

  v13 = 0;
  v14 = 0;
  v15 = 0;
  v16 = 0;

  // 读取偏移检验值（最后一位）
  v5 = *(_BYTE *)(a2 + 19);

    // 计算种子映射哈希（输入a3, 长度16）
  v17 = compute_data_hash(a3, 0x10u);

  // 生成数字映射表（顺序->值），结果保存在 v23[10]
  generate_digit_permutation_map(v17 + v5, (int)v23);

  // 逆向映射：将前19字节数据值反推映射序号
  for ( i = 0; i < 19; ++i )
  {
    for ( j = 0; j < 10; ++j )
    {
      if ( *(unsigned __int8 *)(i + a2) == (unsigned __int8)v23[j] )
      {
        v22[i] = j;
        v22[i + 19] = j;
      }
    }
  }

  // 旋转，还原数据顺序；起始偏移 = 3 + v5
  for ( k = 0; k < 19; ++k )
    v22[k] = v22[k + 3 + v5];

  // 提取字段：产品码/日期/尾数/校验？
  v18 = 10 * v22[1]
      + v22[0]
      + 100 * v22[2]
      + 1000 * v22[3]
      + 10000 * v22[4]
      + 100000 * v22[5]
      + 1000000 * v22[6]
      + 10000000 * v22[7];

  v21 = 10 * v22[9] + v22[8] + 100 * v22[10] + 1000 * v22[11];
  v20 = 10 * v22[13] + v22[12] + 100 * v22[14] + 1000 * v22[15];
  v19 = 10 * v22[17] + v22[16] + 100 * v22[18];

  // 计算重新编码哈希（以辨别数据一致性）
  v9 = compute_data_hash((int)v22, 0x10u);

  // 校验 19 位和尾偏移
  v10 = 0;
  for ( m = 0; m < 19; ++m )
    v10 += (unsigned __int8)v22[m];

  if ( v10 % 0xA == v5 )
  {
    if ( v9 == v19 )
    {
      v14 = v18;
      v15 = v21;
      v16 = v20;
    }
    else
    {
      v13 = 2; // 内部校验失败
    }
    return sub_80141EC(a1, &v13, 0x10u);
  }
  else
  {
    v13 = 1; // 校验和失败
    return sub_80141EC(a1, &v13, 0x10u);
  }
}
// 计算以 data_ptr 为起点、长度 len 的字节序列的滚动哈希（基于 digit_hash_step）
// 该哈希用于映射及校验，等效于 sub_803D986
unsigned int __fastcall compute_data_hash(int data_ptr, unsigned int len)
{
  unsigned int hash; // r8
  unsigned int i; // r6

  hash = 97;                           // 初始值 97
  for ( i = 0; i < len; ++i )
    hash = digit_hash_step(*(unsigned __int8 *)(i + data_ptr), hash);
  return hash;
}

// 单字节滚动哈希：对一个字节值按十进制拆位（0-255），计算 991*(hash+digit)%997
// 等价于 sub_803D950 / sub_532618
unsigned int __fastcall digit_hash_step(unsigned int byte_value, unsigned int current_hash)
{
  unsigned int result; // r0
  unsigned int digit; // r3

  result = current_hash;
  while ( byte_value )
  {
    digit = byte_value % 10;
    byte_value /= 10;
    result = 991 * (digit + result) % 997;
  }
  return result;
}
//----- (080141EC) --------------------------------------------------------
// copy_memory_optimized
// 将源缓冲 a2 中的 a3 字节复制到目标缓冲 result。
// 机制：
//  1) 先按 16 字节（4 个DWORD）块复制
//  2) 然后按 8/4/2/1 字节依次补复制剩余部分
//  3) 最终返回 result 的末尾位置（按 DWORD 指针）
// 用途：用于 decode_and_verify_registration_payload 结束填充输出结构
_DWORD *__fastcall sub_80141EC(_DWORD *result, int *a2, unsigned int a3)
{
  bool v3; // cf
  unsigned int i; // r2
  int v5; // r3
  int v6; // r4
  int v7; // r5
  int v8; // r12
  int v9; // r3
  int v10; // r12
  int v11; // t1
  __int16 v12; // t1

  v3 = a3 >= 0x10;
  for ( i = a3 - 16; v3; result += 4 )
  {
    v5 = *a2;
    v6 = a2[1];
    v7 = a2[2];
    v8 = a2[3];
    a2 += 4;
    v3 = i >= 0x10;
    i -= 16;
    *result = v5;
    result[1] = v6;
    result[2] = v7;
    result[3] = v8;
  }
  if ( __CFSHL__(i, 29) )
  {
    v9 = *a2;
    v10 = a2[1];
    a2 += 2;
    *result = v9;
    result[1] = v10;
    result += 2;
  }
  if ( (i & 4) != 0 )
  {
    v11 = *a2++;
    *result++ = v11;
  }
  if ( __CFSHL__(i, 31) )
  {
    v12 = *(_WORD *)a2;
    a2 = (int *)((char *)a2 + 2);
    *(_WORD *)result = v12;
    result = (_DWORD *)((char *)result + 2);
  }
  if ( (i & 1) != 0 )
    *(_BYTE *)result = *(_BYTE *)a2;
  return result;
}
//----- (08037762) --------------------------------------------------------
// validate_and_apply_registration_code
// a1: 输入注册码字符串指针（要求 ASCII数字、20 字符）
// a2,a3,a4: 资源和上下文参数（序列号/产品相关）
// 返回：0=非法/失败,1=普通有效,2=特殊签名有效
int __fastcall validate_and_apply_registration_code(int reg_code_ptr, int resource_key, int param3, int param4)
{
  _BYTE *decoded_digits; // r2
  int tmp; // r3
  int i; // r1
  unsigned __int8 *resource_data; // r0
  int resource_handle; // r5
  int oem_hash; // r6
  int curr; // r0
  int arg1; // r1
  int arg2; // r2
  int arg3; // r3
  int limit_seconds; // r0
  int a; // r1
  int b; // r2
  int c; // r3
  int result_code; // [sp+0h] [bp-54h] BYREF
  int status_field; // [sp+4h] [bp-50h]
  unsigned int timeout_max; // [sp+8h] [bp-4Ch]
  unsigned int timeout_val; // [sp+Ch] [bp-48h]
  _BYTE candidate_digits[32]; // [sp+10h] [bp-44h] BYREF
  _BYTE key_map[36]; // [sp+30h] [bp-24h] BYREF

  // 先尝试从资源加载一个映射/校验缓冲
  if ( !process_machine_code_record((int)key_map, resource_key, param3, param4) )
    return 0;  // 资源加载失败直接返回无效

  // 校验并转换注册码字符串为数字[0-9]
  for ( i = 0; i < 20; ++i )
  {
    unsigned char ch = *(unsigned __int8 *)(i + reg_code_ptr);
    if ( ch < '0' || ch > '9' )
      return 0;
    candidate_digits[i] = ch - '0';
  }

  // 从资源池获取句柄
  resource_data = (unsigned __int8 *)sub_8014E04((int)dword_2000CFD8, i, (int)candidate_digits, tmp);
  resource_handle = (int)resource_data;
  if ( !resource_data )
    return 0;

  oem_hash = parse_number_string(resource_data, 8u);

  // 核心验证+解码
  decode_and_verify_registration_payload(&result_code, (int)candidate_digits, (int)key_map);
  if ( result_code )
    return 0;

  // 特殊签名：v21==74838438 表示“注册码可用最高权限”
  if ( status_field == 74838438 )
  {
    memcpy_bytes((int)"74838438", resource_handle, 8u);
    ++*(_DWORD *)(resource_handle + 16);
    curr = sub_8014DD0((int)dword_2000CFD8);
    sub_8026294(curr, a, b, c);
    set_license_timeout(0x337E4680u);
    if ( sub_802701E() )
      sub_8037936((int)&unk_2000D954, timeout_max, timeout_val);
    return 2;
  }
  else
  {
    // 非特殊签名路径，做抗篡改/一致性检查
    if ( !sub_8019940(resource_handle, (int)"74838438", 8u) && byte_2000D140 )
    {
      if ( status_field != oem_hash )
        return 0;
    }
    else
    {
      format_number_to_string(status_field, (char *)resource_handle, 8u, 0);
    }

    if ( timeout_val >= 0x2710 )
      timeout_val = 9999;

    ++*(_DWORD *)(resource_handle + 16);
    curr = sub_8014DD0((int)dword_2000CFD8);
    sub_8026294(curr, a, b, c);
    sub_803788A(86400 * timeout_val);
    if ( sub_802701E() )
      sub_8037936((int)&unk_2000D954, timeout_max, timeout_val);
    return 1;
  }
}
//----- (08019C70) --------------------------------------------------------
// parse_number_string
// 根据文本内容将字符数组转换成整数：支持
// - 0x 开头的16进制（大写/小写）
// - 小于9位的纯十进制
// 超过9位的十进制返回错误值300000000
int __fastcall parse_number_string(unsigned __int8 *str, unsigned __int8 len)
{
  int result; // r6
  unsigned __int8 i; // r3
  unsigned __int8 ch; // r5
  unsigned __int8 digit; // r4
  unsigned __int8 j; // r3

  result = 0;
  // 16进制格式支持 0x/0X 开头
  if ( *str == '0' && (str[1] == 'x' || str[1] == 'X') )
  {
    for ( i = 2; i < (unsigned int)len; ++i )
    {
      ch = str[i];
      if ( !ch || result >> 24 )
        break;
      if ( ch >= '0' && ch <= '9' )
        digit = ch - '0';
      else if ( ch >= 'A' && ch <= 'F' )
        digit = ch - 'A' + 10;
      else if ( ch >= 'a' && ch <= 'f' )
        digit = ch - 'a' + 10;
      else
        break;
      result = (16 * result) | digit;
    }
    return result;
  }
  // 十进制解析（容错至少于9位）
  else if ( *str < '3' || len < 9u )
  {
    for ( j = 0; j < (unsigned int)len; ++j )
      result = (str[j] - '0') + 10 * result;
    return result;
  }
  else
  {
    return 300000000;
  }
}
//----- (0803788A) --------------------------------------------------------
// set_license_timeout
// a1: 目标计时长度（秒）
// 功能：如果 a1 小于 0x337E4680（86400*3650?约10年），则开启标志 byte_2000D140=1，
// 否则设置大范围最大值 863913600（约 27.4 年）并关闭标志(byte_2000D140=0)
// 同时写入全局变量并触发相关资源更新（sub_80154D6/sub_8014DD4/sub_8026294）
int __fastcall set_license_timeout(unsigned int timeout_seconds)
{
  int v1; // r1
  int v2; // r2
  int v3; // r3
  int v4; // r0
  int v5; // r1
  int v6; // r2
  int v7; // r3

  dword_2000D144 = timeout_seconds;
  if ( timeout_seconds < 0x337E4680 )
  {
    byte_2000D140 = 1;  // 允许计时到期
  }
  else
  {
    // 超过最大阈值时设置为固定最大值并关闭“定时有效”标志
    dword_2000D144 = 863913600;
    byte_2000D140 = 0;
  }

  sub_80154D6(dword_2000D144);
  *(_BYTE *)(sub_8014DD4((int)dword_2000CFD8, v1, v2, v3) + 434) = byte_2000D140;
  v4 = sub_8014DD0((int)dword_2000CFD8);
  sub_8026294(v4, v5, v6, v7);
  return 1;
}
//----- (08014DD0) --------------------------------------------------------
int __fastcall sub_8014DD0(int a1)
{
  return *(_DWORD *)(a1 + 8);
}
//----- (8019A1E) --------------------------------------------------------
// format_number_to_string
// a1: 要格式化的数值（可负）
// a2: 目标缓冲区
// a3: 长度（输出字符串长度限制）
// a4: 处理方式：0=固定宽补0,1=左移并删除前导0
// 返回: 实际写入的字符数（包含符号）或a3
int __fastcall format_number_to_string(int value, char *out_buf, unsigned __int8 width, char mode)
{
  unsigned __int8 length_written; // r8
  int temp_val; // lr
  unsigned __int8 negative_flag; // r12
  char default_char; // r7
  unsigned __int8 i; // r5
  unsigned __int8 shift_pos; // r6
  unsigned __int8 j; // r5

  length_written = 0;
  temp_val = value;
  negative_flag = 0;
  default_char = *out_buf;
  if ( value < 0 )
  {
    temp_val = -value;
    negative_flag = 1;
  }
  for ( i = width; i; --i )
  {
    if ( temp_val )
    {
      ++length_written;
      out_buf[i - 1] = temp_val % 10 + '0';
      temp_val /= 10;
    }
    else if ( default_char == ' ' )
    {
      out_buf[i - 1] = ' ';
    }
    else
    {
      out_buf[i - 1] = '0';
    }
  }
  if ( !length_written )
    length_written = 1;
  shift_pos = width - length_written - negative_flag;
  if ( mode == 1 )
  {
    if ( shift_pos )
    {
      for ( j = 0; j < (unsigned int)length_written; ++j )
        out_buf[j + negative_flag] = out_buf[shift_pos + negative_flag + j];
      if ( negative_flag + length_written < width )
        out_buf[negative_flag + length_written] = 0;
    }
  }
  if ( negative_flag )
  {
    *out_buf = '-';
    ++length_written;
  }
  if ( mode )
    return length_written;
  else
    return width;
}
  v4 = 0;
  v5 = a1;
  v6 = 0;
  v7 = *a2;
  if ( a1 < 0 )
  {
    v5 = -a1;
    v6 = 1;
  }
  for ( i = a3; i; --i )
  {
    if ( v5 )
    {
      ++v4;
      a2[i - 1] = v5 % 10 + 48;
      v5 /= 10;
    }
    else if ( v7 == 32 )
    {
      a2[i - 1] = 32;
    }
    else
    {
      a2[i - 1] = 48;
    }
  }
  if ( !v4 )
    v4 = 1;
  v9 = a3 - v4 - v6;
  if ( a4 == 1 )
  {
    if ( v9 )
    {
      for ( j = 0; j < (unsigned int)v4; ++j )
        a2[j + v6] = a2[v9 + v6 + j];
      if ( v6 + v4 < a3 )
        a2[v6 + v4] = 0;
    }
  }
  if ( v6 )
  {
    *a2 = 45;
    ++v4;
  }
  if ( a4 )
    return v4;
  else
    return a3;
}
//----- (08017304) --------------------------------------------------------
int __fastcall sub_8017304(_DWORD *a1, unsigned int a2, int a3)
{
  unsigned int v6; // r7

  v6 = a1[4] >> 9;
  if ( a2 < v6 )
  {
    if ( !sub_8017466((int)a1, a2 << 9, 0) )
      return 0;
  }
  else
  {
    while ( v6 < a2 )
    {
      if ( !sub_80174E0((int)a1) )
        return 0;
      ++v6;
    }
  }
  return sub_80172AA(a1, a3, 0);
}
//----- (08026270) --------------------------------------------------------
int __fastcall sub_8026270(int a1, unsigned __int8 a2, int a3, int a4)
{
  if ( a2 != *(unsigned __int8 *)(a1 + 520) )
  {
    *(_BYTE *)(a1 + 520) = a2;
    sub_8017304(*(_DWORD **)a1, a2, a1 + 8);
  }
  return a4;
}
//----- (0802624C) --------------------------------------------------------
int __fastcall sub_802624C(int a1, int a2, int a3, int a4)
{
  sub_8026270(a1, 2u, a3, a4);
  return a1 + 8;
}
//----- (08014E04) --------------------------------------------------------
// 资源获取函数
// 功能：从资源池中获取指定ID的资源
// 参数：
//   resource_pool: 资源池指针
//   resource_id: 资源ID
//   param1, param2: 附加参数
// 返回值：成功返回资源句柄，失败返回0
int __fastcall get_resource(int resource_pool, int resource_id, int param1, int param2)
{
  int resource_handle; // 资源句柄

  // 检查资源池是否有效
  if ( !*(_DWORD *)(resource_pool + 8) )
    return 0;
  
  // 调用资源获取函数
  resource_handle = sub_802624C(*(_DWORD *)(resource_pool + 8), resource_id, param1, param2);
  
  // 检查资源获取是否成功
  if ( !resource_handle )
    return 0;
  
  return resource_handle;
}

// 数学计算函数
// 功能：执行复杂的浮点数计算，可能与物理模型相关
// 参数：
//   data_struct: 包含多个浮点数字段的结构体指针
// 返回值：特殊浮点数值，可能表示计算状态
int __fastcall calculate_math_function(int data_struct)
{
  float temp_value; // 临时计算值
  
  // 复杂的浮点数条件判断
  // 涉及多个字段的计算，可能是某种物理模型的约束条件
  if ( (float)((float)-(float)((float)(*(float *)(data_struct + 28) * 2.0) * *(float *)(data_struct + 16))
             - (float)((float)-(float)(*(float *)(data_struct + 12) * *(float *)(data_struct + 12))
                     - (float)((float)-(float)(*(float *)(data_struct + 4) * *(float *)(data_struct + 4))
                             - (float)((float)(*(float *)(data_struct + 8) * 2.0) * *(float *)(data_struct + 8))))) <= 0.0 )
  {
    // 条件满足时的处理
    *(_BYTE *)(data_struct + 32) = 1; // 设置标志位
    
    // 计算并存储结果
    *(float *)(data_struct + 36) = (float)(*(float *)(data_struct + 8) - *(float *)(data_struct + 4)) / *(float *)(data_struct + 28);
    
    // 复杂的计算过程
    *(float *)(data_struct + 40) = (float)((float)((float)((float)((float)((float)((float)-(float)((float)(*(float *)(data_struct + 8)
                                                                                                * 2.0)
                                                                                        * *(float *)(data_struct + 8))
                                                                        - (float)((float)(*(float *)(data_struct + 28) * 2.0)
                                                                                * *(float *)(data_struct + 16)))
                                                                + (float)(*(float *)(data_struct + 4) * *(float *)(data_struct + 4)))
                                                        + (float)(*(float *)(data_struct + 12) * *(float *)(data_struct + 12)))
                                                / 2.0)
                                        / *(float *)(data_struct + 28))
                                / *(float *)(data_struct + 8))
                        + *(float *)(data_struct + 36);
    
    // 进一步计算
    *(float *)(data_struct + 44) = (float)((float)(*(float *)(data_struct + 8) - *(float *)(data_struct + 12)) / *(float *)(data_struct + 28))
                        + *(float *)(data_struct + 40);
    
    // 设置特殊值（可能表示负无穷）
    *(_DWORD *)(data_struct + 48) = -1082130432;
    *(_DWORD *)(data_struct + 52) = -1082130432;
    
    return -1082130432;
  }
  else
  {
    // 条件不满足时的处理
    *(_BYTE *)(data_struct + 32) = 0; // 清除标志位
    
    // 计算临时值
    temp_value = (float)(*(float *)(data_struct + 28) * *(float *)(data_struct + 16))
       + (float)((float)((float)(*(float *)(data_struct + 4) * *(float *)(data_struct + 4))
                       + (float)(*(float *)(data_struct + 12) * *(float *)(data_struct + 12)))
               / 2.0);
    
    // 调用辅助函数
    sub_802052C();
    
    // 存储计算结果
    *(float *)(data_struct + 56) = temp_value;
    *(float *)(data_struct + 36) = (float)(*(float *)(data_struct + 56) - *(float *)(data_struct + 4)) / *(float *)(data_struct + 28);
    *(float *)(data_struct + 40) = (float)((float)(*(float *)(data_struct + 56) - *(float *)(data_struct + 12)) / *(float *)(data_struct + 28))
                        + *(float *)(data_struct + 36);
    
    // 设置特殊值
    *(_DWORD *)(data_struct + 48) = -1082130432;
    
    return -1082130432;
  }
}