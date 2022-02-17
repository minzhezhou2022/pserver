#include <mio/all.h>
#include <gtest/gtest.h> 

namespace {
TEST (Shell, file_read) {
  auto fp = mio::shell_open_file("test_shell_input.txt", "r", 8192);
  mio::LineFileReader linereader;
  std::string line0 = std::string(linereader.getline(&*fp));
  
  EXPECT_EQ("this is line 0", line0);
}

TEST (Shell, open_pipe_write) {
  {
    auto fpo = mio::shell_open_pipe("cat > test_shell_output.txt", "w", 8192);
    std::string result = "test shell open\n";
    fwrite_unlocked((void*)result.c_str(), result.length(), 1, &*fpo);
  }
  auto fpi = mio::shell_open_file("test_shell_output.txt", "r", 8192);
 
  mio::LineFileReader linereader;
  std::string line = std::string(linereader.getline(&*fpi));
  EXPECT_EQ("test shell open", line);
}

TEST (Shell, open_pipe_read) {
  auto fpi = mio::shell_open_pipe("cat test_shell_output.txt", "r", 8192);
  mio::LineFileReader linereader;
  std::string line = std::string(linereader.getline(&*fpi));
  EXPECT_EQ("test shell open", line);
  mio::localfs_remove("test_shell_output.txt");
}

}
