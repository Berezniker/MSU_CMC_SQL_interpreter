#include "gtest/gtest.h"

#include <server/server.h>

TEST(connection_test, run_server) {
    ASSERT_NO_THROW(Server s(8070, 10));
}