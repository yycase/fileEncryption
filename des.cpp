#include "des.h"

// 生成16轮密钥
void DES::generateKey(bitset<64> key) {
    // 置换1后的56位密钥
    bitset<56> k;
    k = permutation1(key);
    //cout << k << endl << endl;

    // 左部分和右部分
    bitset<28> left, right, newl, newr;
    bit56tobit28(k, left, right);

    for (int i = 0; i < 16; i++) {
        newl = leftShift(left, shiftBits[i]);
        newr = leftShift(right, shiftBits[i]);
        bit28tobit56(k, newl, newr);
        subkey[i] = permutation2(k);
        left = newl, right = newr;
    }
}

// 代替函数组S
bitset<32> DES::subS(bitset<48> sin) {
    bitset<32> output;
    int x = 0;
    for (int i = 0; i < 48; i += 6) {
        // 进行S盒转化
        int row = sin[i] * 2 + sin[i + 5];
        int col = sin[i + 1] * 8 + sin[i + 2] * 4 + sin[i + 3] * 2 + sin[i + 4];
        bitset<4> num = sbox[i / 6][row * 12 + col];

        // 赋值给32位output
        for (int j = 0; j < 4; j++)
            output[x + j] = num[3 - j];
        x += 4;
    }
    return output;
}


// 选择运算E
bitset<48> DES::selectE(bitset<32> part) {
    bitset<48> res;
    for (int i = 0; i < 48; i++)
        res[i] = part[E[i] - 1];
    return res;
}

// 置换运算P
bitset<32> DES::permuP(bitset<32> inp) {
    bitset<32> res;
    for (int i = 0; i < 32; i++)
        res[i] = inp[P[i] - 1];
    return res;
}

// 56位分为28位和28位
void DES::bit56tobit28(bitset<56>& res, bitset<28>& left, bitset<28>& right) {
    for (int i = 0; i < 28; i++) {
        left[i] = res[i];
        right[i] = res[i + 28];
    }

}

// 28位合成56位
void DES::bit28tobit56(bitset<56>& res, bitset<28>& left, bitset<28>& right) {
    for (int i = 0; i < 28; i++) {
        res[i] = left[i];
        res[i + 28] = right[i];
    }
}


// 置换选择1
bitset<56> DES::permutation1(bitset<64> key) {
    bitset<56> res;
    for (int i = 0; i < 56; i++)
        res[i] = key[PC_1[i] - 1];
    return res;
}

// 循环左移
bitset<28> DES::leftShift(bitset<28> key, int tag) {
    return key >> tag | key << (28 - tag);
}

// 置换选择2
bitset<48> DES::permutation2(bitset<56> key) {
    bitset<48> res;
    for (int i = 0; i < 48; i++)
        res[i] = key[PC_2[i] - 1];
    return res;
}

// 初始置换
bitset<64> DES::initialPermutation(bitset<64> init) {
    bitset<64> res;
    for (int i = 0; i < 64; i++) {
        res[i] = init[IP[i] - 1];
    }
    return res;
}

bitset<64> DES::invInitialPermutation(bitset<64> init) {
    bitset<64> res;
    for (int i = 0; i < 64; i++) {
        res[i] = init[IP_1[i] - 1];
    }
    return res;
}

// 64位变为32位
void DES::bit64tobit32(bitset<64>& res, bitset<32>& left, bitset<32>& right) {
    for (int i = 0; i < 32; i++) {
        left[i] = res[i];
        right[i] = res[i + 32];
    }
}

// 32位合为64位
void DES::bit32tobit64(bitset<64>& res, bitset<32>& left, bitset<32>& right) {
    for (int i = 0; i < 32; i++) {
        res[i] = left[i];
        res[i + 32] = right[i];
    }
}

// 左右32位互换
void DES::exchangelr(bitset<32>& left, bitset<32>& right) {
    bitset<32> temp;
    temp = left;
    left = right;
    right = temp;
}

// 加密函数f
bitset<32> DES::f(bitset<32> in, bitset<48> key) {
    // 选择运算E
    bitset<48> temp;
    temp = selectE(in);

    // 与子密钥异或
    bitset<48> mid;
    mid = temp ^ key;

    // S盒变换
    bitset<32> cur;
    cur = subS(mid);

    // 置换运算P
    bitset<32> later;
    later = permuP(cur);
    return later;

}

// split a string
template<typename Out>
void DES::split(const string &s, char delim, Out result)
{
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim))
        *result++ = item;
}

vector<string> DES::split(const string &s, char delim)
{
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

// 加密
bitset<64> DES::encrypt(bitset<64> plain) {
    bitset<64> res;
    bitset<64> oIP;
    bitset<64> invIP;
    bitset<32> left;
    bitset<32> right;
    bitset<32> newleft;



    // 初始置换
    oIP = initialPermutation(plain);

    // 将64位变为32位
    bit64tobit32(oIP, left, right);

    for (int i = 0; i < 16; i++) {
        newleft = right;
        right = left ^ f(right, subkey[i]);
        left = newleft;
    }


    // 将32位合成64位 左右要互换
    bit32tobit64(invIP, right, left);

    // 逆初始置换
    res = invInitialPermutation(invIP);

    return res;


}

// 解密
bitset<64> DES::decrypt(bitset<64> cipher) {
    bitset<64> res;
    bitset<64> oIP;
    bitset<64> invIP;
    bitset<32> left;
    bitset<32> right;
    bitset<32> newleft;



    // 初始置换
    oIP = initialPermutation(cipher);

    // 将64位变为32位
    bit64tobit32(oIP, left, right);

    for (int i = 15; i >= 0; i--) {
        newleft = right;
        right = left ^ f(right, subkey[i]);
        left = newleft;
    }

    // 将32位合成64位 左右要互换
    bit32tobit64(invIP, right, left);

    // 逆初始置换
    res = invInitialPermutation(invIP);

    return res;

}


// 字符串转二进制
bitset<64> DES::char2bits(const char s[8]) {
    //0001110011101100011011001010110000101100110011000100110010001100
    bitset<64> bits;
    int x = 0;
    for (int i = 0; i < 8; i++) {
        int num = (int)s[i];
        bitset<8> temp(num);
        for (int j = 0; j < 8; j ++)
            bits[x + j] = temp[7 - j];
        x += 8;
    }
    return bits;
}


// 将连续的64位分成8组，存到一个 byte 数组中
void DES::divideToByte(byte out[8], bitset<64>& data)
{
    bitset<64> temp;
    for (int i = 0; i < 8; ++i)
    {
        temp = (data << 8 * i) >> 56;
        out[i] = temp.to_ulong();
    }
}

// 将8个 byte 合并成连续的64位
bitset<64> DES::mergeByte(byte in[8])
{
    bitset<64> res;
    res.reset();  // 置0
    bitset<64> temp;
    for (int i = 0; i < 8; ++i)
    {
        temp = in[i].to_ulong();
        temp <<= 8 * (7 - i);
        res |= temp;
    }
    return res;
}


// 加密文件
string DES::encryptFile(string inPath, string outPath) {
    bitset<64> data;
    bitset<64> key;
    bitset<64> outfile;

    ifstream in;
    ofstream out;

    // 生成16个子密钥
    key = mergeByte(innerKey);
    generateKey(key);

    //保存文件的路径
    vector<string> x = split(inPath, '/');
    string ends = x[x.size()-1];
    string savePath = outPath + "/des_En_" + ends;

    in.open(inPath, ios::binary);
    out.open(savePath, ios::binary);

    // 移动文件指针到文件末尾，得到文件的长度后，复原文件指针
    in.seekg(0, ios::end);
    int length = in.tellg(), cur;
    in.seekg(0, ios::beg);

    while (in.read((char*)&data, sizeof(data)))
    {

        outfile = encrypt(data);
        out.write((char*)&outfile, sizeof(outfile));

        // 找到目前文件指针所在位置，判断剩余文件流是否大于等于一个data字节大小，小于的话直接跳出循环
        cur = in.tellg();
        if (cur + 8 > length) break;

        data.reset();  // 置0
        outfile.reset();
    }

    // 得到剩余文件流字节大小
    int num = length - cur;
    //初始化一段内存
    char rest[8] = "1";
    // 将剩余的文件读入到该内存中
    in.read(rest, num);
    // 将读入内存的部分写入另一个文件
    out.write(rest, num);


    in.close();
    out.close();

    return savePath;

}

// 解密文件
string DES::decryptFile(string inPath, string outPath) {
    bitset<64> data;
    bitset<64> key;
    bitset<64> outfile;

    ifstream in;
    ofstream out;

    // 生成16个子密钥
    key = mergeByte(innerKey);
    generateKey(key);

    vector<string> x = split(inPath, '/');
    string ends = x[x.size()-1];
    string savePath = outPath + "/des_De_" + ends;

    in.open(inPath, ios::binary);
    out.open(savePath, ios::binary);

    // 移动文件指针到文件末尾，得到文件的长度后，复原文件指针
    in.seekg(0, ios::end);
    int length = in.tellg(), cur;
    in.seekg(0, ios::beg);

    while (in.read((char*)&data, sizeof(data)))
    {
        outfile = decrypt(data);
        out.write((char*)&outfile, sizeof(outfile));

        // 找到目前文件指针所在位置，判断剩余文件流是否大于等于一个data字节大小，小于的话直接跳出循环
        cur = in.tellg();
        if (cur + 8 > length) break;

        data.reset();  // 置0
        outfile.reset();
    }

    // 得到剩余文件流字节大小
    int num = length - cur;
    //初始化一段内存
    char rest[8] = "1";
    // 将剩余的文件读入到该内存中
    in.read(rest, num);
    // 将读入内存的部分写入另一个文件
    out.write(rest, num);

    in.close();
    out.close();

    return savePath;

}
