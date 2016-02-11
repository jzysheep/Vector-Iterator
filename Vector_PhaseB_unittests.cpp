/*
 * Vector_PhaseB_unittests.cpp
 * EE380L - Spring 2015
 *
 * Tests for Vector_PhaseB are organized into three sections: PhaseB and PhaseB1
 * correspond to the B and B* requirements respectively. These tests are independent,
 * so you may comment out the ones that you are not utilizing, and Google Test will
 * run accordingly.
 *
 * These tests are not complete. Write additional tests on your own to test
 * the rest of the functionality of your program. The tests used to grade your
 * project will be more robust than those included in this file.
*/

#include <iostream>
#include "gtest/gtest.h"
#include "Vector.h"

using std::cout;
using std::endl;
using epl::vector;

/*****************************************************************************************/
// Class Instrumentation
/*****************************************************************************************/
namespace {
    //Class Instrumentation
    class Foo {
    public:
        bool alive;

        static uint64_t constructions;
        static uint64_t destructions;
        static uint64_t copies;
        static uint64_t moves;
        static void reset() { moves = copies = destructions = constructions = 0; }

        Foo(void) { alive = true; ++constructions; }
        ~Foo(void) { if(alive) destructions += 1; }
        Foo(const Foo&) noexcept { alive = true; ++copies; }
        Foo(Foo&& that) noexcept { that.alive = false; this->alive = true; ++moves; }
    };

    uint64_t Foo::constructions = 0;
    uint64_t Foo::destructions = 0;
    uint64_t Foo::copies = 0;
    uint64_t Foo::moves = 0;
} //namespace

/*****************************************************************************************/
// Phase B Tests
/*****************************************************************************************/
#if defined(PHASE_B0_0) | defined(PHASE_B)
TEST(PhaseB, MoveCtor) {
    vector<Foo> x;
    for (unsigned int i = 0; i < 10; ++i) {
        x.push_back(Foo());
    }

    vector<Foo> y(x);
    vector<Foo> z(std::move(x));

    EXPECT_EQ(y.size(), z.size());
}
#endif

#if defined(PHASE_B1_0) | defined(PHASE_B)
TEST(PhaseB1, PushBackMove) {
    Foo::reset();
    {
        vector<Foo> x(10); // 10 default-constructed Foo objects
        for (int k = 0; k < 11; ++k) {
            x.push_back(Foo());
        }
    } //ensures x is destroyed

    EXPECT_EQ(21, Foo::constructions);
    EXPECT_EQ(21, Foo::destructions);
    EXPECT_EQ(0, Foo::copies);
    EXPECT_LE(21, Foo::moves);
}
#endif

/*
 * There are no official B** requirements, but this could be
 * considered a B** test.
 */
#if defined(PHASE_B2_0) | defined(PHASE_B)
TEST(PhaseB1, ReallocCopy)
{
    Foo::reset();
    {
        vector<vector<Foo>> x(8);
        x[0].push_front(Foo()); //1 alive Foo
        x.push_back(x[0]); //1 copy, 2 alive Foo
    } //ensures x is destroyed

    EXPECT_EQ(1, Foo::constructions);
    EXPECT_EQ(2, Foo::destructions);
    EXPECT_EQ(1, Foo::copies);
    EXPECT_GE(2, Foo::moves);
}
#endif
