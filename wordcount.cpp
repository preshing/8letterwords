#define READ_WHOLE_FILE 1
#define PRINT_RESULTS 0

#include <vector>
#include <map>
#include <unordered_map>
#include <fstream>
#include <chrono>
#include <iostream>


//---------------------------------------------------------------------
// String8
//---------------------------------------------------------------------

class String8
{
private:
    union
    {
        unsigned __int64 m_value;
        char m_chars[8];
    };

public:
    String8() : m_value(0) {}

    String8(const char* from)
    {
        memcpy(m_chars, from, 8);
    }

    char* getBuf()
    {
        return m_chars;
    }

    std::string toString() const
    {
        char buf[9];
        memcpy(buf, m_chars, 8);
        buf[8] = 0;
        return buf;
    }

    operator bool() const
    {
        return m_value != 0;
    }

    bool operator<(const String8& other) const
    {
        return m_value < other.m_value;
    }

    bool operator==(const String8& other) const
    {
        return m_value == other.m_value;
    }

    unsigned __int64 getValue() const
    {
        return m_value;
    }
};


//---------------------------------------------------------------------
// Hash function for String8 so we can use it in std::unordered_map
//---------------------------------------------------------------------

namespace std
{
    template <> struct hash<String8>
    {
        size_t operator()(const String8& k) const
        {
            return hash<unsigned __int64>()(k.getValue());
        }
    };
}


//---------------------------------------------------------------------
// WordReader
//---------------------------------------------------------------------
#if READ_WHOLE_FILE

class WordReader
{
private:
    std::vector<char> m_wholeFile;
    char *m_rdPos;
    char *m_endPos;

public:
    WordReader(const char* filename) : m_rdPos(nullptr), m_endPos(nullptr)
    {
        std::ifstream inFile(filename);
        if (!inFile.fail())
        {
            inFile.seekg(0, std::ios::end);
            size_t fileSize = inFile.tellg();
            inFile.seekg(0, std::ios::beg);

            m_wholeFile.resize(fileSize);
            inFile.read(&m_wholeFile[0], fileSize);
            m_rdPos = &m_wholeFile[0];
            m_endPos = m_rdPos + fileSize;
        }
    }

    String8 getWord();
};

String8 WordReader::getWord()
{
    String8 result;
    char* buf = result.getBuf();
    int wordLen = 0;
    while (m_rdPos < m_endPos)
    {
        char c = *m_rdPos++;
        if (c >= 'a' && c <= 'z')
        {
            if (wordLen < 8)
                buf[wordLen++] = (char) c;
        }
        else
        {
            if (wordLen > 0)
                return result;
        }
    }
    return result;
}

#else

class WordReader
{
private:
    std::ifstream m_inFile;

public:
    WordReader(const char* filename) : m_inFile(filename) {}

    String8 getWord();
};

String8 WordReader::getWord()
{
    String8 result;
    char* buf = result.getBuf();
    int wordLen = 0;
    for (;;)
    {
        int c = m_inFile.get();
        if (c == EOF)
            break;
        if (c >= 'a' && c <= 'z')
        {
            if (wordLen < 8)
                buf[wordLen++] = (char) c;
        }
        else
        {
            if (wordLen > 0)
                return result;
        }
    }
    return result;
}

#endif


//---------------------------------------------------------------------
// TestLinearSearch
//---------------------------------------------------------------------

struct Item
{
    String8 word;
    int count;

    Item(const String8& w, int c) : word(w), count(c) {}
};

void TestLinearSearch(WordReader& reader)
{
    std::vector<Item> wordList;

    while (String8 word = reader.getWord())
    {
        int i;
        for (i = 0; i < wordList.size(); i++)
        {
            if (wordList[i].word == word)
            {
                wordList[i].count++;
                break;
            }
        }
        if (i >= wordList.size())
        {
            wordList.emplace_back(word, 1);
        }
    }

#if PRINT_RESULTS
    for (const Item& item : wordList)
    {
        std::cout << item.word.toString().c_str() << ": " << item.count << "\n";
    }
#endif
}


//---------------------------------------------------------------------
// TestStdMap
//---------------------------------------------------------------------

void TestStdMap(WordReader& reader)
{
    std::map<String8, int> wordCount;

    while (String8 word = reader.getWord())
    {
        wordCount[word] += 1;   // New entries are autoinitialized to 0
    }

#if PRINT_RESULTS
    for (const auto& item : wordCount)
    {
        std::cout << item.first.toString().c_str() << ": " << item.second << "\n";
    }
#endif
}


//---------------------------------------------------------------------
// TestStdUnorderedMap
//---------------------------------------------------------------------

void TestStdUnorderedMap(WordReader& reader)
{
    std::unordered_map<String8, int> wordCount;

    while (String8 word = reader.getWord())
    {
        wordCount[word] += 1;   // New entries are autoinitialized to 0
    }

#if PRINT_RESULTS
    for (const auto& item : wordCount)
    {
        std::cout << item.first.toString().c_str() << ": " << item.second << "\n";
    }
#endif
}


//---------------------------------------------------------------------
// main
//---------------------------------------------------------------------

struct TestEntry
{
    void (*testFunc)(WordReader& reader);
    const char *name;
};

TestEntry TestEntries[] =
{
    { TestLinearSearch, "linear search" },
    { TestStdMap, "std::map" },
    { TestStdUnorderedMap, "std::unordered_map" },
};

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        puts("Please specify the input filename on the command line.");
        return 1;
    }

    for (TestEntry& entry : TestEntries)
    {
        auto start = std::chrono::high_resolution_clock::now();
        WordReader reader(argv[1]);
        entry.testFunc(reader);
        auto end = std::chrono::high_resolution_clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << entry.name << ": " << millis << " ms\n";
    }

    return 0;
}
