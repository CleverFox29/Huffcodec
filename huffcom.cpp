#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <fstream>
#include <queue>
#include <cstdint>
#include <vector>

struct node
{
    uint64_t f = 0;
    char data = '\0';
    node *left = nullptr;
    node *right = nullptr;
};
struct compare
{
    bool operator()(node *a, node *b)
    {
        return a->f > b->f;
    }
};

void bunch_duplicates_frequencies(std::priority_queue<node *, std::vector<node *>, compare> &pq)
{
    std::priority_queue<node *, std::vector<node *>, compare> temp;
    bool dup = true;
    while (dup)
    {
        dup = false;
        while (!pq.empty())
        {

            node *i = pq.top();
            pq.pop();
            std::vector<node *> v;
            v.push_back(i);
            while (!pq.empty() && pq.top()->f == i->f)
            {
                v.push_back(pq.top());
                pq.pop();
                dup = true;
            }
            int n = v.size();

            std::vector<node *> tree;
            tree.reserve(2 * n - 1);
            tree.assign(n - 1, nullptr);
            tree.insert(tree.end(), v.begin(), v.end());

            for (size_t i = n - 1; i-- > 0;)
            {
                node *&t = tree[i];
                t = new node();
                t->left = tree[2 * i + 1];
                t->right = tree[2 * i + 2];
                t->f = t->left->f + t->right->f;
            }
            temp.push(tree[0]);
        }
        std::swap(pq, temp);
    }
}

int cnt = 0;

void map_path(node *head, std::map<char, std::string> &huffmap, std::ofstream &out, std::string path = "", bool root = true)
{
    if (!head)
        return;

    if (!head->left && !head->right)
    {
        std::cout << head->data << ": " << head->f << "  \t" << path << std::endl;
        out << 1 << head->data;
        huffmap[head->data] = path;
        cnt++;
    }
    else
    {
        out << 0;
    }
    map_path(head->left, huffmap, out, path + "0", false);
    map_path(head->right, huffmap, out, path + "1", false);
    if (root)
    {
        std::cout << cnt << " unique characters encoded" << std::endl;
        cnt = 0;
    }
}

int compress(std::string input, std::string output)
{
    std::ifstream in(input,std:: ios:: binary);
    std::map<char, uint64_t> mp;

    std::priority_queue<node *, std::vector<node *>, compare> pq;

    if (!in.is_open())
    {
        return 1; // file failed to open
    }

    // creating frequency table;
    char c;
    while (in.get(c))
    {
        mp[c]++;
    }

    // table to nodes;
    int count = 0;
    for (auto &i : mp)
    {
        node *t = new node();
        t->data = i.first;
        t->f = i.second;
        pq.push(t);
        std::cout << t->data << ": " << t->f << std::endl;
        count++;
    }
    std::cout << count << " unique characters found." << std::endl;

    // checkout for the twist;
    bunch_duplicates_frequencies(pq);

    // build tree;
    while (pq.size() > 1)
    {
        node *a = pq.top();
        pq.pop();
        node *b = pq.top();
        pq.pop();

        node *t = new node();
        t->f = a->f + b->f;
        t->left = a;
        t->right = b;
        pq.push(t);
    }
    if (pq.size() == 0)
    {
        pq.push(new node());
    }

    node *head = pq.top();

    uint64_t file_size = head->f;
    std::ofstream out(output,std::ios::binary);
    if (!out.is_open())
    {
        return 2;
    }
    out.write(reinterpret_cast<char *>(&file_size), sizeof(file_size));

    // for quick encoding
    std::map<char, std::string> huffmap;

    // populates huffman also writes map to file in preorder
    map_path(head, huffmap, out);

    in.clear();
    in.seekg(0);

    while (in.get(c))
    {
        for (char i : huffmap[c])
        {
            if(i == '1'){

            }else if(i=='0'){

            }
        }
    }

    return 0;
}

node *readtree(std::ifstream &in)
{
    node *t = new node();
    char c;
    if (!in.get(c))
        return nullptr;
    if (c == '0')
    {
        t->left = readtree(in);
        t->right = readtree(in);
    }
    else if (c == '1')
    {
        if (!in.get(c))
            return nullptr;
        t->data = c;
    }
    return t;
}

int decompress(std::string input, std::string output)
{
    std::ifstream in(input,std::ios::binary);
    if (!in.is_open())
    {
        return 3;
    }
    std::ofstream out(output,std::ios::binary);
    if (!out.is_open())
    {
        return 4;
    }

    uint64_t file_size = 0;

    in.read(
        reinterpret_cast<char *>(&file_size),
        sizeof(file_size));

    node *head = readtree(in);

    // for quick encoding
    std::map<char, std::string> huffmap;

    // populates huffman also writes map to file in preorder
    map_path(head, huffmap, out);

    return 0;
}

int main()
{
    std::string input = "gita.txt";
    std::string output = "gita(cmp).huff";
    std::string recovery = "gita-recovered.txt";

    int status = compress(input, output);

    std::cout << std::endl;
    if (status == 0)
    {
        std::cout << "OK";
    }
    else
    {
        if (status == 1)
        {
            std::cout << "compressor input file error"<<std::endl;
        }
        else if (status == 2)
        {
            std::cout << "compressor output file error"<<std::endl;
        }
        else if (status == 3)
        {
            std::cout << "decompressor input file error"<<std::endl;
        }
        else
            std::cout << "Error"<<std::endl;
    }

    status = decompress(output, recovery);

    std::cout << std::endl;
    if (status == 0)
    {
        std::cout << "OK"<<std::endl;
    }
    else
    {
        if (status == 1)
        {
            std::cout << "compressor input file error"<<std::endl;
        }
        else if (status == 2)
        {
            std::cout << "compressor output file error"<<std::endl;
        }
        else if (status == 3)
        {
            std::cout << "decompressor input file error"<<std::endl;
        }
        else
            std::cout << "Error"<<std::endl;
    }
}
