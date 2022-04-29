#include "aes.h"

// 分割字符
using std::getline;
using std::back_inserter;

// 字符串工具
using std::stringstream;
using std::hex;

// 文件流
using std::ifstream;
using std::ofstream;
using std::ios;

// 64位密钥
uint64_t key;
// 存放16轮子密钥
uint64_t subKey[16];

/**
 *  密码函数f，接收32位数据和48位子密钥，产生一个32位的输出
 */
uint64_t f(uint64_t R, uint64_t K)
{
    uint64_t expandR = 0;

    // E 变换 32 -> 48
    for(int i=0; i<48; ++i)
        expandR |= (((R >> (31-E[i])) & 0x1) << (47-i));

    // R XOR K
    expandR ^= K;

    // 查找S_BOX置换表
    uint64_t output = 0;
    int x = 0;
    uint64_t mask = 63;// 每次用低六位
    int index = 0;
    uint64_t num = 0;
    for(int i=0; i<8; ++i)
    {
        index = expandR & mask;
        expandR = expandR >> 6;
        num = S_BOX2[7-i][index];
        output |= (num << x);
        x += 4;
    }
    // P-置换，32 -> 32
    uint64_t tmp = output;
    output = 0;
    for(int i=0; i<32; ++i)
        output |= (((tmp >> (31-P[i])) & 0x1) << (31-i));

    return output;
}

/**
 *  对56位密钥的前后部分进行左移
 */
void leftShift(uint64_t &k, int shift)
{
    k = ((k << shift) | (k >> (28 - shift))) & 0x0fffffff;
}

/**
 *  生成16个48位的子密钥
 */
void generateKeys()
{
    uint64_t realKey = 0;
    uint64_t left = 0;
    uint64_t right = 0;
    uint64_t compressKey = 0;

    for (int i=0; i<56; ++i)
        realKey |= (((key >> (63-PC_1[i])) & 0x1) << (55-i));

    // 生成子密钥，保存在 subKeys[16] 中
    for(int round=0; round<16; ++round)
    {
        // 前28位与后28位
        left = realKey >> 28;
        right = realKey & 0xfffffff;
        // 左移
        leftShift(left, shiftBits[round]);
        leftShift(right, shiftBits[round]);
        // 压缩置换，由56位得到48位子密钥
        realKey = (left << 28) | right;
        // 第五步：结尾置换FP
        for(int i=0; i<48; ++i)
            compressKey |= (((realKey >> (55-PC_2[i])) & 0x1) << (47-i));

        subKey[round] = compressKey;
        compressKey = 0;
    }
}

/**
 *  工具函数
 */
// 将16进制字符串转为2进制uint_64
uint64_t Hex2Bin(string s)
{
    stringstream ss;
    ss << hex << s;
    uint64_t n;
    ss >> n;
    return n;
}

// 将2进制uint_64转为16进制字符串
string Bin2Hex(uint64_t s)
{
    stringstream ss;
    ss << hex << s;
    return ss.str();
}

// split a string
template<typename Out>
void split(const string &s, char delim, Out result)
{
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim))
        *result++ = item;
}

vector<string> split(const string &s, char delim)
{
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

uint64_t crypt(uint64_t &input, bool ed)
{
    uint64_t output = 0;
    uint64_t currentBits = 0;
    uint64_t left = 0;
    uint64_t right = 0;
    uint64_t newLeft = 0;
    // 第一步：初始置换IP
    for(int i=0; i<64; ++i)
        currentBits |= (((input >> (63-IP[i])) & 0x1) << (63-i));

    // 第二步：获取 Li 和 Ri
    left = currentBits >> 32;
    right = currentBits & 0xffffffff;
    // 第三步：共16轮迭代
    if (ed == true)// ed == true, 加密
    {
        for(int round=0; round<16; ++round)
        {
            newLeft = right;
            right = left ^ f(right, subKey[round]);
            left = newLeft;
        }
    }
    else// ed == false, 解密
    {
        for(int round=0; round<16; ++round)
        {
            newLeft = right;
            right = left ^ f(right,subKey[15-round]);
            left = newLeft;
        }
    }

    // 第四步：合并L16和R16，注意合并为 R16L16
    output = left | (right << 32);
    // 第五步：结尾置换FP
    currentBits = output;
    output = 0;
    for(int i=0; i<64; ++i)
        output |= (((currentBits >> (63-FP[i])) & 0x1) << (63-i));
    // 返回明文／密文
    return output;
}

// 加密
uint64_t encrypt(uint64_t &plain)
{
    return crypt(plain, true);
}

// 解密
uint64_t decrypt(uint64_t &cipher)
{
    return crypt(cipher, false);
}


// 计算文件大小
std::streampos fileSize(string path)
{
    ifstream in;
    in.open(path, ios::binary);
    in.seekg(0, std::ios_base::end);
    std::streampos sp = in.tellg();
    in.close();
    return sp;
}

// 加密文件
string encryptFile(string inPath, string outPath)
{

    ifstream in;
    ofstream out;
    uint64_t sp = fileSize(inPath);// 文件字节数
    vector<string> x = split(inPath, '/');
    string ends = x[x.size()-1];// ends表示原文件名
    string savePath = outPath + "/aes_en_" + ends;
    in.open(inPath, ios::binary);
    out.open(savePath, ios::binary);
    uint64_t plain = 0, cipher = 0;
    int round = sp/8;
    for(int i=0; i < round; ++i)//对8的整数倍的字节进行正常加密
    {
        in.read((char*)&plain, 8);
        cipher  = encrypt(plain);
        out.write((char*)&cipher, 8);
    }

    in.read((char*)&plain, sp%8);
    cipher  = encrypt(plain);
    out.write((char*)&cipher, 8);

    in.read((char*)&plain, sp%8);
    cipher = encrypt(sp);
    out.write((char*)&cipher, 8);

    in.close();
    out.close();

    return savePath;
}

// 解密文件
string decryptFile(string inPath, string outPath)
{

    ifstream in;
    ofstream out;
    uint64_t cipher = 0, plain = 0;
    in.open(inPath, ios::binary);
    uint64_t sp = fileSize(inPath);// 文件字节数
    vector<string> x = split(inPath, '/');
    string ends = x[x.size()-1];
    string savePath = outPath + "/aes_de_" + ends;
    out.open(savePath, ios::binary);
    int round = sp/8;
    for(int i=0; i<round-2; ++i)// 解密时注意循环上界
    {
        in.read((char*)&cipher, 8);
        plain = decrypt(cipher);
        out.write((char*)&plain, 8);
    }

    uint64_t tmp = 0, file_size = 0;
    in.read((char*)&cipher, 8);
    tmp = decrypt(cipher);
    in.read((char*)&cipher, 8);
    file_size = decrypt(cipher);
    out.write((char*)&tmp, file_size % 8);

    in.close();
    out.close();

    return savePath;
}
