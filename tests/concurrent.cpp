#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helper.h"
#include "../run.h"
#include "../mm.h"

extern "C" {
    void __tsan_on_report() {
        FAIL() << "Encountered a thread sanitizer error";
    }
}

using ::testing::MatchesRegex;

TEST(Concurent, CounterIncs) {
    uint const THREADS = 8;
    uint const OPS = 1000;
    testing::internal::CaptureStdout();
    run(prog({
        "obj = object",
        repeat(THREADS, "t", {
            repeat(OPS, "o", {"var_$t_$o = object"}),
        }),
        "dump obj",
        repeat(THREADS, "t", {
            "thread {",
                repeat(OPS, "o", {"var_$t_$o = obj"}),
            "}",
        }),
        "sleep",
        "dump obj",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump obj: strong\(\w+\), obj refCounter = 1, fields = \{\}\s*)--"
        R"--(dump obj: strong\(\w+\), obj refCounter = 1, fields = \{\}\s*)--"
    ));
}

TEST(Concurent, CounterDecs) {
    uint const THREADS = 8;
    uint const OPS = 1000;
    testing::internal::CaptureStdout();
    run(prog({
        "obj = object",
        repeat(THREADS, "t", {
            repeat(OPS, "o", {"var_$t_$o = obj"}),
        }),
        "dump obj",
        repeat(THREADS, "t", {
            "thread {",
                repeat(OPS, "o", {"var_$t_$o = object"}),
            "}",
        }),
        "sleep",
        "dump obj",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    auto counterBefore = std::to_string(THREADS * OPS + 1);
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump obj: strong\(\w+\), obj refCounter = )--" + counterBefore + R"--(, fields = \{\}\s*)--"
        R"--(dump obj: strong\(\w+\), obj refCounter = 1, fields = \{\}\s*)--"
    ));
}

TEST(Concurent, CounterIncsDesc) {
    uint const INC_THREADS = 4;
    uint const DEC_THREADS = 4;
    uint const OPS = 1000;
    testing::internal::CaptureStdout();
    run(prog({
        "obj = object",
        repeat(INC_THREADS, "t", {
            repeat(OPS, "o", {"var_inc_$t_$o = object"}),
        }),
        repeat(DEC_THREADS, "t", {
            repeat(OPS, "o", {"var_dec_$t_$o = obj"}),
        }),
        "dump obj",
        repeat(INC_THREADS, "t", {
            "thread {",
            repeat(OPS, "o", {"var_inc_$t_$o = obj"}),
            "}",
        }),
        repeat(DEC_THREADS, "t", {
            "thread {",
            repeat(OPS, "o", {"var_dec_$t_$o = object"}),
            "}",
        }),
        "sleep",
        "dump obj",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    auto counterBefore = std::to_string(INC_THREADS * OPS + 1);
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump obj: strong\(\w+\), obj refCounter = )--" + counterBefore + R"--(, fields = \{\}\s*)--"
        R"--(dump obj: strong\(\w+\), obj refCounter = 1, fields = \{\}\s*)--"
    ));
}

TEST(Concurent, WeakCounterIncsDesc) {
    uint const INC_THREADS = 4;
    uint const DEC_THREADS = 4;
    uint const OPS = 1000;
    testing::internal::CaptureStdout();
    run(prog({
        "obj = object",
        "weak ~= obj",
        repeat(INC_THREADS, "t", {
            repeat(OPS, "o", {"var_inc_$t_$o = object"}),
        }),
        repeat(DEC_THREADS, "t", {
            repeat(OPS, "o", {"var_dec_$t_$o ~= weak"}),
        }),
        "dump weak",
        repeat(INC_THREADS, "t", {
            "thread {",
            repeat(OPS, "o", {"var_inc_$t_$o ~= weak"}),
            "}",
        }),
        repeat(DEC_THREADS, "t", {
            "thread {",
            repeat(OPS, "o", {"var_dec_$t_$o = object"}),
            "}",
        }),
        "sleep",
        "dump weak",
        "dump obj", // keep obj alive
    }));
    std::string output = testing::internal::GetCapturedStdout();
    auto weakCounterBefore = std::to_string(INC_THREADS * OPS + 2);
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump weak: weak\(\w+ -> \w+\), weak refCounter = )--" + weakCounterBefore + R"--(, obj refCounter = 1, fields = \{\}\s*)--"
        R"--(dump weak: weak\(\w+ -> \w+\), weak refCounter = 2, obj refCounter = 1, fields = \{\}\s*)--"
        R"--(dump obj: strong\(\w+\), obj refCounter = 1, fields = \{\}\s*)--"
    ));
}

#pragma clang diagnostic pop