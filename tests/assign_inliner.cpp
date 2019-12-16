#include "verilogAST/assign_inliner.hpp"
#include "common.cpp"
#include "gtest/gtest.h"

namespace vAST = verilogAST;

class AssignInliner : public vAST::Transformer {
 public:
  using vAST::Transformer::visit;
};

namespace {
TEST(InlineAssignTests, TestBasic) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input i, output o);\n"
      "wire x;\n"
      "assign x = i;\n"
      "assign o = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (input i, output o);\n"
      "assign o = i;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestBasicChain) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("y"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("y")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input i, output o);\n"
      "wire x;\n"
      "wire y;\n"
      "assign x = i;\n"
      "assign y = x;\n"
      "assign o = y;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (input i, output o);\n"
      "assign o = i;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestBasicExpr) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      vAST::make_binop(std::make_unique<vAST::Identifier>("i"),
                       vAST::BinOp::XOR,
                       std::make_unique<vAST::NumericLiteral>("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input i, output o);\n"
      "wire x;\n"
      "assign x = i ^ 1;\n"
      "assign o = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (input i, output o);\n"
      "assign o = i ^ 1;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestExprChain) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o"), vAST::OUTPUT,
                                               vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("y")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      vAST::make_binop(std::make_unique<vAST::Identifier>("i"),
                       vAST::BinOp::XOR,
                       std::make_unique<vAST::NumericLiteral>("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("y"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o"),
      std::make_unique<vAST::Identifier>("y")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input i, output o);\n"
      "wire x;\n"
      "wire y;\n"
      "assign x = i ^ 1;\n"
      "assign y = x;\n"
      "assign o = y;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (input i, output o);\n"
      "assign o = i ^ 1;\n"
      "endmodule\n";

  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestInlineFanOutId) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o0"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o1"),
                                               vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      std::make_unique<vAST::Identifier>("i")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o0"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o1"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input i, output o0, output o1);\n"
      "wire x;\n"
      "assign x = i;\n"
      "assign o0 = x;\n"
      "assign o1 = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str =
      "module test_module (input i, output o0, output o1);\n"
      "assign o0 = i;\n"
      "assign o1 = i;\n"
      "endmodule\n";
  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

TEST(InlineAssignTests, TestNoInlineFanOutExpr) {
  std::vector<std::unique_ptr<vAST::AbstractPort>> ports;
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("i"), vAST::INPUT,
                                               vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o0"),
                                               vAST::OUTPUT, vAST::WIRE));
  ports.push_back(std::make_unique<vAST::Port>(vAST::make_id("o1"),
                                               vAST::OUTPUT, vAST::WIRE));

  std::vector<std::variant<std::unique_ptr<vAST::StructuralStatement>,
                           std::unique_ptr<vAST::Declaration>>>
      body;

  body.push_back(
      std::make_unique<vAST::Wire>(std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("x"),
      vAST::make_binop(std::make_unique<vAST::Identifier>("i"),
                       vAST::BinOp::XOR,
                       std::make_unique<vAST::NumericLiteral>("1"))));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o0"),
      std::make_unique<vAST::Identifier>("x")));

  body.push_back(std::make_unique<vAST::ContinuousAssign>(
      std::make_unique<vAST::Identifier>("o1"),
      std::make_unique<vAST::Identifier>("x")));

  std::unique_ptr<vAST::AbstractModule> module = std::make_unique<vAST::Module>(
      "test_module", std::move(ports), std::move(body));

  std::string raw_str =
      "module test_module (input i, output o0, output o1);\n"
      "wire x;\n"
      "assign x = i ^ 1;\n"
      "assign o0 = x;\n"
      "assign o1 = x;\n"
      "endmodule\n";

  EXPECT_EQ(module->toString(), raw_str);

  std::string expected_str = raw_str;
  vAST::AssignInliner transformer;
  EXPECT_EQ(transformer.visit(std::move(module))->toString(), expected_str);
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
