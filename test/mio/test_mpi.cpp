// mzhou@pinterest 2021-12-15
#include <mio/all.h>
#include <gtest/gtest.h> 


namespace {
static std::string real_ip = mio::trim_spaces(mio::shell_get_command_output("hostname -I"));
static int mpi_argc = 0;
static char** mpi_argv = new char*[8];

TEST(MPI, test_mpi_check_consistency) {
  int x;
  MPI_Init(&mpi_argc, &mpi_argv);
  mio::mpi_check_consistency(&x, 100);
}

TEST (MPI, test_mpi_get_local_ip_internal) {
  std::string result = mio::mpi_get_local_ip_internal();
  EXPECT_EQ(real_ip, result);
}

TEST(MPI, test_mpi_ip) {
  mio::mpi_init_internal();
  std::string result = mio::mpi_ip();
  EXPECT_EQ(real_ip, result);
}

}