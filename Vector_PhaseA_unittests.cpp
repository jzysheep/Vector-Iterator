/*
 * Vector_PhaseA_unittests.cpp
 * EE380L - Spring 2015
 * 
 * Tests for Vector_PhaseA are organized into three sections: PhaseA, PhaseA1,
 * and PhaseA2 correspond to the A, A*, and A** requirements respectively.
 * These tests are independent, so you may comment out the ones that you are
 * not utilizing, and Google Test will run accordingly.
 *
 * These tests are not complete. Write additional tests on your own to test
 * the rest of the functionality of your program. The tests used to grade your
 * project will be more robust than those included in this file.
 */

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "gtest/gtest.h"
#include "Vector.h"

using std::cout;
using std::endl;

using epl::vector;

// TEST SUITE A
TEST(PhaseA, push_back)
{
    vector<int> x;
    EXPECT_EQ(0, x.size());

    x.push_back(42);
    EXPECT_EQ(1, x.size());
    EXPECT_EQ(42, x[0]);
}

TEST(PhaseA, bracket_operator)
{
    vector<int> x;
    EXPECT_EQ(0, x.size());

    x.push_back(42);
    EXPECT_EQ(1, x.size());
    EXPECT_EQ(42, x[0]);

    x[0] = 10;
    EXPECT_EQ(10, x[0]);
}

TEST(PhaseA, pop_back)
{
    vector<int> x;
    EXPECT_EQ(0, x.size());

    x.push_back(42);
    EXPECT_EQ(1, x.size());

    x.pop_back();
    EXPECT_EQ(0, x.size());
}

TEST(PhaseA, constructors)
{
    vector<int> x;
    EXPECT_EQ(0, x.size());

    x.push_back(42);
    vector<int> y{ x };
    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y[0]);

    y[0] = 10;
    EXPECT_NE(10, x[0]);

    vector<int> z(10); // must use () to avoid ambiguity over initializer list
    EXPECT_EQ(10, z.size());
}

TEST(PhaseA, range_checks)
{
    vector<int> x(10);
    EXPECT_NO_THROW(
    for (int k = 0; k < 10; k += 1) {
        x[k] = k;
    });

    EXPECT_THROW(x[10] = 42, std::out_of_range);
}

// TEST SUITE A*
// Ghetto C-style way to find the size of an array
// Only used in next test
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(*X))
TEST(PhaseA1, PushBackFront)
{
    vector<int> x;
    EXPECT_EQ(0, x.size());

    x.push_back(42);
    EXPECT_EQ(1, x.size());

    for (int k = 0; k < 10; k += 1) {
        x.push_back(k);
        x.push_front(k);
    }

    int ans[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 42, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    EXPECT_EQ(ARRAY_SIZE(ans), x.size());
    for (uint64_t i = 0; i < ARRAY_SIZE(ans); i += 1)
    {
        EXPECT_EQ(x[i], ans[i]);
    }
}

TEST(PhaseA1, CopyConstruct)
{
    vector<int> x;
    x.push_back(42);
    vector<int> y(x); // copy constructed
    x.push_back(0);

    EXPECT_EQ(1, y.size());
    EXPECT_EQ(42, y[0]);

    y.pop_back();
    EXPECT_EQ(0, y.size());
    EXPECT_EQ(2, x.size());
    EXPECT_EQ(42, x[0]);
    EXPECT_EQ(0, x[1]);
}

// TEST SUITE A**
namespace
{
    // Class Instrumentation
    class Foo
    {
    public:
        bool alive;

        static uint64_t constructions;
        static uint64_t destructions;
        static uint64_t copies;
        static void reset() { copies = destructions = constructions = 0; }

        Foo(void) { alive = true; ++constructions; }
        ~Foo(void) { if(alive) destructions += 1; }
        Foo(const Foo&) { alive = true; ++copies; }
        Foo(Foo&& that) { that.alive = false; this->alive = true; }
        // Foo& operator=(const Foo& that) { alive=true; copies += 1; return *this; }
        // Foo& operator=(Foo&& that) noexcept { that.alive = false; this->alive = true; return *this; }
    };

    uint64_t Foo::constructions = 0;
    uint64_t Foo::destructions = 0;
    uint64_t Foo::copies = 0;

} // end empty namespace

TEST(PhaseA2, FooCtorDtor)
{
    Foo::reset();
    {
        vector<Foo> x(10); // 10 default-constructed Foo objects
        for (int k = 0; k < 11; ++k) {
            x.push_front(Foo()); // default-construct temp, then move it
        }
    } //ensures x is destroyed

    EXPECT_EQ(21, Foo::constructions);
    EXPECT_GE(62, Foo::destructions);
    EXPECT_GE(41, Foo::copies);
}

/* I'm offering no guidance here, other than this is a case you should explore */
TEST(PhaseA2, ReallocCopy)
{
    Foo::reset();
    {
        vector<vector<Foo>> x(3);
        x.pop_front();
        x[0].push_front(Foo()); //1 alive Foo
        x.push_front(x[0]); //1 copy, 2 alive Foo
    } //ensures x is destroyed

    EXPECT_EQ(1, Foo::constructions);
    EXPECT_GE(3, Foo::destructions);
    EXPECT_GE(2, Foo::copies);
}

/*
 * This is the main entry point for the program.  Other
 * tests can be in other cpp files, as long as there is
 * only one of these main functions.
 */
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto out = RUN_ALL_TESTS();
#ifdef _MSC_VER
    system("pause");
#endif
    return out;
}
