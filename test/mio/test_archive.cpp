// mzhou@pinterest 2021-12-15

#include <mio/all.h>
#include <gtest/gtest.h> 


namespace {

TEST(Archive, test_archive_base) {
    mio::BinaryArchive ar;
    char buf[100] = {0};
    ar.set_buffer(buf, 0, 100, nullptr);
    EXPECT_EQ(ar.position(), 0);
    EXPECT_EQ(ar.length(), 0);
    EXPECT_EQ(ar.capacity(), 100);
    EXPECT_EQ(ar.empty(), true);

    std::vector<int> v = {1, 2, 3, 4};
    ar.write((void *)&v[0], sizeof(int) * v.size());
    EXPECT_EQ(ar.position(), 0);
    EXPECT_EQ(ar.length(), sizeof(int) * v.size());
    EXPECT_EQ(ar.capacity(), 100);

    int x;
    ar.read((void *)&x, sizeof(int));
    EXPECT_EQ(x, 1);
    EXPECT_EQ(ar.position(), sizeof(int));
    EXPECT_EQ(ar.length(), sizeof(int) * v.size());
    EXPECT_EQ(ar.capacity(), 100);
    EXPECT_EQ(ar.empty(), false);

    x = ar.template get_raw<int>();
    EXPECT_EQ(x, 2);
    EXPECT_EQ(ar.position(), sizeof(int) * 2);
    EXPECT_EQ(ar.length(), sizeof(int) * v.size());
    EXPECT_EQ(ar.capacity(), 100);

    ar.read_back((void *)&x, sizeof(int));
    EXPECT_EQ(x, 4);
    EXPECT_EQ(ar.position(), sizeof(int) * 2);
    EXPECT_EQ(ar.length(), sizeof(int) * (v.size() - 1));
    EXPECT_EQ(ar.capacity(), 100);

    ar.template get_raw<int>(x);
    EXPECT_EQ(x, 3);
    EXPECT_EQ(ar.position(), sizeof(int) * 3);
    EXPECT_EQ(ar.length(), sizeof(int) * (v.size() - 1));
    EXPECT_EQ(ar.capacity(), 100);
}

TEST(Archive, test_reserve_write) {
    mio::BinaryArchive ar;
    constexpr size_t init_sz = 16;
    char buf[init_sz] = {0};
    ar.set_buffer(buf, 0, init_sz, nullptr);
    EXPECT_EQ(ar.position(), 0);
    EXPECT_EQ(ar.length(), 0);
    EXPECT_EQ(ar.capacity(), init_sz);
    EXPECT_EQ(ar.empty(), true);


    ar.template put_raw<int>(0);
    EXPECT_EQ(ar.position(), 0);
    EXPECT_EQ(ar.length(), 4);
    EXPECT_EQ(ar.capacity(), init_sz);
    EXPECT_EQ(ar.empty(), false);


    std::vector<int> v = {1, 2, 3, 4};
    ar.write((void *)&v[0], sizeof(int) * v.size());
    EXPECT_EQ(ar.position(), 0);
    EXPECT_EQ(ar.length(), 4 + sizeof(int) * v.size());
    EXPECT_EQ(ar.capacity(), init_sz * 2);
    EXPECT_EQ(ar.empty(), false);
}

TEST(Archive, test_text_vector) {
    std::vector<int> in= {1,3,7};
    std::vector<int> out;
    mio::TextArchive ar;
    ar << in;
    ar >> out;
    EXPECT_EQ(in, out);
}

TEST(Archive, test_binary_map) {
    std::map<int, std::string> in= {
        {1, "Jack"},
        {2, "Jim"}
    };
    std::map<int, std::string> out;
    mio::BinaryArchive ar;
    ar << in;
    ar >> out;
    EXPECT_EQ(in, out);
}

}