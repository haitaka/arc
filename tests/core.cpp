#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "helper.h"
#include "../run.h"

// FIXME throw from a non-main thread can not be caught

using ::testing::MatchesRegex;

TEST(Lang, VarAssignIsInit) {
    run("x = object");
}

TEST(Lang, FieldAssignIsInit) {
    run(prog({
        "x = object",
        "y = object",
        "y.field = object",
    }));
}

TEST(Lang, UninitVarIsErr) {
    ASSERT_ANY_THROW(
        run("x = y")
    );
}

TEST(Lang, UninitFieldIsErr) {
    ASSERT_ANY_THROW(
        run(prog({
            "x = object",
            "y = x.field",
        }));
    );
}

TEST(Lang, ThreadCanAccessGlobal) {
    run(prog({
        "x = object",
        "thread {",
            "y = x",
        "}",
    }));
}

TEST(Lang, ThreadCanCreateGlobal) {
    run(prog({
        "thread {",
            "x = object",
        "}",
        "sleep",
        "y = x",
    }));
}

TEST(Lang, ThreadCanAccessForeignGlobal) {
    run(prog({
        "thread {",
            "x = object",
        "}",
        "sleep",
        "thread {",
            "y = x",
        "}",
    }));
}

TEST(Lang, AssignmentIncrementsCounter) {
    testing::internal::CaptureStdout();
    run(prog({
        "x = object",
        "dump x",
        "y = x",
        "dump x",
        "dump y",
        "anchor ~= x", // do not let `x` die too early
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump x: strong\(\w+\), obj refCounter = 1, fields = \{\}\s+)--"
        R"--(dump x: strong\(\w+\), obj refCounter = 2, fields = \{\}\s+)--"
        R"--(dump y: strong\(\w+\), obj refCounter = 2, fields = \{\}\s*)--"
        ));
}

TEST(Lang, WeakAssignmentDoesNotIncremetCounter) {
    testing::internal::CaptureStdout();
    run(prog({
        "x = object",
        "dump x",
        "y ~= x",
        "dump x",
        "dump y",
        "anchor ~= x", // do not let `x` die too early
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump x: strong\(\w+\), obj refCounter = 1, fields = \{\}\s+)--"
        R"--(dump x: strong\(\w+\), obj refCounter = 1, fields = \{\}\s+)--"
        R"--(dump y: weak\(\w+ -> \w+\), weak refCounter = 2, obj refCounter = 1, fields = \{\}\s*)--"
    ));
}

TEST(Lang, GlobalDiesAtLastUse) {
    testing::internal::CaptureStdout();
    run(prog({
        "x = object",
        "dump x",
        "y = x",
        "dump x",
        // x dies here
        "dump y",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump x: strong\(\w+\), obj refCounter = 1, fields = \{\}\s+)--"
        R"--(dump x: strong\(\w+\), obj refCounter = 2, fields = \{\}\s+)--"
        R"--(dump y: strong\(\w+\), obj refCounter = 1, fields = \{\}\s*)--"
    ));
}

TEST(Lang, GlobalDiesWithLastThread) {
    testing::internal::CaptureStdout();
    run(prog({
        "x = object",
        "wx ~= x",
        "thread {",
        "sleep",
        "y ~= x",
        "}",
        "dump wx",
        "sleep",
        "sleep",
        "dump wx",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump wx: weak\(\w+ -> \w+\), weak refCounter = 2, obj refCounter = 1, fields = \{\}\s+)--"
        R"--(dump wx: weak\(\w+ -> 0\), weak refCounter = 1, obj collected\s+)--"
    ));
}

TEST(Lang, WeakRefsAreFreed) {
    testing::internal::CaptureStdout();
    run(prog({
        "x = object",
        "w0 ~= x",
        "w1 ~= x",
        "w2 ~= x",
        "w3 ~= x",
        "w4 ~= x",
        "w5 ~= x",
        "dump w0",
        "dump x",
        "dump w0",
        "dump w1",
        "dump w2",
        "dump w3",
        "dump w4",
        "dump w5",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump w0: weak\(\w+ -> \w+\), weak refCounter = 7, obj refCounter = 1, fields = \{\}\s+)--"
        R"--(dump x: strong\(\w+\), obj refCounter = 1, fields = \{\}\s+)--"
        R"--(dump w0: weak\(\w+ -> 0\), weak refCounter = 6, obj collected\s+)--"
        R"--(dump w1: weak\(\w+ -> 0\), weak refCounter = 5, obj collected\s+)--"
        R"--(dump w2: weak\(\w+ -> 0\), weak refCounter = 4, obj collected\s+)--"
        R"--(dump w3: weak\(\w+ -> 0\), weak refCounter = 3, obj collected\s+)--"
        R"--(dump w4: weak\(\w+ -> 0\), weak refCounter = 2, obj collected\s+)--"
        R"--(dump w5: weak\(\w+ -> 0\), weak refCounter = 1, obj collected\s+)--"
    ));
}

TEST(Lang, FieldsDieWithAnObject) {
    testing::internal::CaptureStdout();
    run(prog({
        "a = object",
        "b = object",
        "c = object",
        "d = object",
        "e = object",
        "f = object",
        "g = object",
        "h = object",
        "i = object",
        "j = object",
        "a.next = b",
        "b.next = c",
        "c.next = d",
        "d.next = e",
        "e.next = f",
        "f.next = g",
        "g.next = h",
        "h.next = i",
        "i.next = j",
        "j.next ~= a",
        "wj ~= j",
        "dump wj",
        "dump a",
        // a dies here
        "dump wj",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump wj: weak\(\w+ -> \w+\), weak refCounter = 2, obj refCounter = 1, fields = \{next: weak\(\w+ -> \w+\)\}\s+)--"
        R"--(dump a: strong\(\w+\), obj refCounter = 1, fields = \{next: strong\(\w+\)\}\s+)--"
        R"--(dump wj: weak\(\w+ -> 0\), weak refCounter = 1, obj collected\s*)--"
    ));
}

TEST(Lang, TheCircleOfAlive) {
    testing::internal::CaptureStdout();
    run(prog({
        "a = object",
        "b = object",
        "c = object",
        "d = object",
        "e = object",
        "f = object",
        "g = object",
        "h = object",
        "i = object",
        "j = object",
        "a.next = b",
        "b.next = c",
        "c.next = d",
        "d.next = e",
        "e.next = f",
        "f.next = g",
        "g.next = h",
        "h.next = i",
        "i.next = j",
        "j.next = a",
        "dump a",
        "wa ~= a",
        "dump wa",
    }));
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_THAT(output, MatchesRegex(
        R"--(dump a: strong\(\w+\), obj refCounter = 2, fields = \{next: strong\(\w+\)\}\s+)--"
        R"--(dump wa: weak\(\w+ -> \w+\), weak refCounter = 2, obj refCounter = 1, fields = \{next: strong\(\w+\)\}\s+)--"
    ));
}

#pragma clang diagnostic pop