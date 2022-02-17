#include <mio/all.h>
#include <gtest/gtest.h> 

namespace {

TEST (FS, fs_open_read) {
  mio::s3_set_command("aws s3");
  auto localfp = mio::fs_open_read("test_shell_input.txt");
  auto s3fp = mio::fs_open_read("s3://datausers/mzhou/test_data/test_shell_input.txt");
  mio::LineFileReader linereader;
  std::string local_line = std::string(linereader.getline(&*localfp));
  std::string s3_line = std::string(linereader.getline(&*s3fp));
  EXPECT_EQ(local_line, s3_line);
}

TEST (FS, fs_open_write) {
  mio::s3_set_command("aws s3");
  std::string test_file_name = "s3://datausers/mzhou/test_data/test_shell_output.txt";
  std::string result = "fs_open_write";
  {
    auto wfp = mio::fs_open_write(test_file_name);
    fwrite_unlocked((void*)result.c_str(), result.length(), 1, &*wfp);
  }
  auto rfp = mio::fs_open_read(test_file_name);
  mio::LineFileReader linereader;
  std::string line = std::string(linereader.getline(&*rfp));
  EXPECT_EQ(result, line);
  mio::fs_remove(test_file_name);
}

TEST (FS, fs_list) {
  mio::s3_set_command("aws s3");
  std::vector<std::string> result = mio::fs_list("s3://datausers/mzhou/test_data/");
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], "s3://datausers/mzhou/test_data/test_shell_input.gz");
  EXPECT_EQ(result[1], "s3://datausers/mzhou/test_data/test_shell_input.txt");
}

TEST (FS, fs_tail) {
  mio::s3_set_command("aws s3");
  std::string local_tail = mio::fs_tail("test_shell_input.txt");
  std::string s3_tail = mio::fs_tail("s3://datausers/mzhou/test_data/test_shell_input.txt");
  EXPECT_EQ(local_tail, s3_tail);
}

TEST (FS, fs_exists) {
  mio::s3_set_command("aws s3");
  EXPECT_EQ(true, mio::fs_exists("s3://datausers/mzhou/test_data/test_shell_input.txt"));
  EXPECT_EQ(false, mio::fs_exists("s3://datausers/mzhou/test_data/non_exist"));
}

TEST (FS, local_isdir) {
  EXPECT_EQ(true, mio::local_isdir("bin"));
  EXPECT_EQ(false, mio::local_isdir("Makefile"));
}

}