#include <gtest/gtest.h>

#include <cudaflags.h>

TEST(CudaFlagsTest, HasFlags)
{
	CudaFlags flags;
	EXPECT_FALSE(flags & CudaFlagBits::CHW || flags & CudaFlagBits::HWC);

	auto flags2 = flags | CudaFlagBits::HWC;

	flags |= CudaFlagBits::CHW;
	EXPECT_TRUE(flags & CudaFlagBits::CHW);

	EXPECT_TRUE(flags2 & CudaFlagBits::HWC);

}


TEST(CudaFlagsTest, RemoveFlag)
{
	CudaFlags flags;
	flags |= CudaFlagBits::CHW;
	flags |= CudaFlagBits::HWC;
	flags |= CudaFlagBits::RGB;

	flags &= ~CudaFlagBits::CHW;
	EXPECT_FALSE(flags & CudaFlagBits::CHW);
	EXPECT_TRUE(flags & CudaFlagBits::HWC);
	EXPECT_TRUE(flags & CudaFlagBits::RGB);
	EXPECT_FALSE(flags & CudaFlagBits::RGBA);
}