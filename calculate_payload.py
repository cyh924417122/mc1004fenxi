# 定义字符集
A_ABCDEFGHJKMNPQRSTUVWXYZ = "ABCDEFGHJKMNPQRSTUVWXYZ"
A_ACDEFGHJKLMNPQRSTUVWXYZ = "ACDEFGHJKLMNPQRSTUVWXYZ"

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