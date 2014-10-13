/* Copyright (c) <2009> <Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "dCILstdafx.h"
#include "dCIL.h"
#include "dDataFlowGraph.h"
#include "dCILInstrBranch.h"
#include "dCILInstrLoadStore.h"
#include "dCILInstrArithmetic.h"
#include "dRegisterInterferenceGraph.h"

#define D_OPTIMIZE_REDUNDANCE_COPY_ON_ARRAYS

//dDataFlowGraph::dDataFlowGraph (dCIL* const cil, dCIL::dListNode* const function, dCIL::dReturnType returnType)
dDataFlowGraph::dDataFlowGraph (dCIL* const cil, dCIL::dListNode* const function)
	:m_mark (0)
//	,m_savedRegistersMask(0)
	,m_cil (cil)
//	,m_returnType(returnType)
	,m_function(function)
	,m_variableDefinitions()
{
}

dDataFlowGraph::~dDataFlowGraph(void)
{
}


void dDataFlowGraph::BuildBasicBlockGraph()
{
	m_basicBlocks.Clear();
	m_basicBlocks.Build (*m_cil, m_function);

	// build leading block map table
	m_dataFlowGraph.RemoveAll();


	dTree<dBasicBlock*, dCIL::dListNode*> blockMap;
	for (dBasicBlocksList::dListNode* blockNode = m_basicBlocks.GetFirst(); blockNode; blockNode = blockNode->GetNext()) {
		dBasicBlock& block = blockNode->GetInfo();

		blockMap.Insert(&block, block.m_begin);

		int test = 1;
		for (dCIL::dListNode* stmtNode = block.m_begin; test; stmtNode = stmtNode->GetNext()) {
			test = (stmtNode != block.m_end);
			dTree<dDataFlowPoint, dCIL::dListNode*>::dTreeNode* const graphNode = m_dataFlowGraph.Insert(stmtNode);
			graphNode->GetInfo().Init(stmtNode);
		} 

		for (dCIL::dListNode* stmtNode = block.m_begin; stmtNode != block.m_end; stmtNode = stmtNode->GetNext()) {
			dDataFlowPoint& graphStatement = m_dataFlowGraph.Find(stmtNode)->GetInfo();
			graphStatement.m_successors.Append(&m_dataFlowGraph.Find(stmtNode->GetNext())->GetInfo());
		}

		for (dCIL::dListNode* stmtNode = block.m_end; stmtNode != block.m_begin; stmtNode = stmtNode->GetPrev()) {
			dDataFlowPoint& graphStatement = m_dataFlowGraph.Find(stmtNode)->GetInfo();
			graphStatement.m_predecessors.Append(&m_dataFlowGraph.Find(stmtNode->GetPrev())->GetInfo());
		}
	}

//m_cil->Trace();
	m_mark += 1;  
	dList<dBasicBlock*> stack;
	stack.Append(&m_basicBlocks.GetFirst()->GetInfo());
	while (stack.GetCount()) {
		dBasicBlock* const block = stack.GetLast()->GetInfo();

		stack.Remove(stack.GetLast()->GetInfo());
		if (block->m_mark < m_mark) {
			block->m_mark = m_mark;
			m_traversalBlocksOrder.Addtop(block);

			dCILInstr* const instruction = block->m_end->GetInfo();
			if (instruction->GetAsIF()) {
				dCILInstrIFNot* const ifInstr = instruction->GetAsIF();
				dAssert (m_dataFlowGraph.Find(block->m_end));
				dAssert (m_dataFlowGraph.Find(ifInstr->GetTrueTarget()));
				dAssert (m_dataFlowGraph.Find(ifInstr->GetFalseTarget()));
				dAssert (blockMap.Find(ifInstr->GetTrueTarget()));
				dAssert (blockMap.Find(ifInstr->GetFalseTarget()));
				dAssert (m_dataFlowGraph.Find(block->m_end->GetNext()));
				dAssert (blockMap.Find(block->m_end->GetNext()));

				dDataFlowPoint* const graphStatement = &m_dataFlowGraph.Find(block->m_end)->GetInfo();
				dDataFlowPoint* const child0 = &m_dataFlowGraph.Find(ifInstr->GetTrueTarget())->GetInfo();
				dDataFlowPoint* const child1 = &m_dataFlowGraph.Find(ifInstr->GetFalseTarget())->GetInfo();

				graphStatement->m_successors.Append(child0);
				graphStatement->m_successors.Append(child1);
				child0->m_predecessors.Append(graphStatement);
				child1->m_predecessors.Append(graphStatement);

				stack.Append(blockMap.Find(ifInstr->GetTrueTarget())->GetInfo());
				stack.Append(blockMap.Find(ifInstr->GetFalseTarget())->GetInfo());

			} else if (instruction->GetAsGoto()) {
				dAssert (0);
				dCILInstrGoto* const gotoInst = instruction->GetAsGoto();

				dAssert (m_dataFlowGraph.Find(block->m_end));
				dAssert (m_dataFlowGraph.Find(gotoInst->GetTarget()));
				dAssert (blockMap.Find(gotoInst->GetTarget()));

				dDataFlowPoint* const graphStatement = &m_dataFlowGraph.Find(block->m_end)->GetInfo();
				dDataFlowPoint* const child0 = &m_dataFlowGraph.Find(gotoInst->GetTarget())->GetInfo();

				graphStatement->m_successors.Append(child0);
				child0->m_predecessors.Append(graphStatement);

				stack.Append(blockMap.Find(gotoInst->GetTarget())->GetInfo());

			} else if (instruction->GetAsReturn()) {
/*
				dAssert (m_dataFlowGraph.Find(block->m_end));
				dAssert (m_dataFlowGraph.Find(block->m_end->GetNext()));
				dAssert (blockMap.Find(block->m_end->GetNext()));

				dDataFlowPoint* const graphStatement = &m_dataFlowGraph.Find(block->m_end)->GetInfo();
				dDataFlowPoint* const child0 = &m_dataFlowGraph.Find(block->m_end->GetNext())->GetInfo();

				graphStatement->m_successors.Append(child0);
				child0->m_predecessors.Append(graphStatement);

				stack.Append(blockMap.Find(block->m_end->GetNext())->GetInfo());
*/
			}
		}
	}
}

#if 0
void dDataFlowGraph::dBasicBlock::Trace() const
{
	#ifdef TRACE_INTERMEDIATE_CODE
		int test = 1;
		for (dCIL::dListNode* stmtNode = m_begin; test; stmtNode = stmtNode->GetNext()) {
			test = (stmtNode != m_end);
			const dThreeAdressStmt& stmt = stmtNode->GetInfo();
			DTRACE_INTRUCTION(&stmt);
		}
		dTrace(("\n"));
	#endif
}

int dDataFlowGraph::EvaluateBinaryExpression (const dString& arg1, dThreeAdressStmt::dOperator operation, const dString& arg2) const
{
	int operando = 0;
	int operando1 = arg1.ToInteger();
	int operando2 = arg2.ToInteger();

	switch (operation) 
	{
		case dThreeAdressStmt::m_add:
			operando = operando1 + operando2;
			break;

		case dThreeAdressStmt::m_greather:
			operando = operando1 > operando2;
			break;

		case dThreeAdressStmt::m_less:
			operando = operando1 < operando2;
			break;

		case dThreeAdressStmt::m_sub:
			operando = operando1 - operando2;
			break;

		case dThreeAdressStmt::m_mul:
			operando = operando1 * operando2;
			break;

		case dThreeAdressStmt::m_div:
			operando = operando1 / operando2;
			break;

		case dThreeAdressStmt::m_mod:
			operando = operando1 % operando2;
			break;

		case dThreeAdressStmt::m_equal:
			operando = (operando1 = operando2);
			break;

		case dThreeAdressStmt::m_identical:
			operando = (operando1 == operando2);
			break;

		case dThreeAdressStmt::m_different:
			operando = (operando1 != operando2);
			break;

		case dThreeAdressStmt::m_lessEqual:
			operando = (operando1 <= operando2);
			break;

		case dThreeAdressStmt::m_greatherEqual:
			operando = (operando1 >= operando2);
			break;

		case dThreeAdressStmt::m_operatorsCount:

		default:
			dAssert (0);
	}
	return operando;
}




bool dDataFlowGraph::ApplyConstantPropagation()
{
	bool ret = false;
	for (dCIL::dListNode* stmtNode = m_function; stmtNode; stmtNode = stmtNode->GetNext()) {
		dThreeAdressStmt& stmt = stmtNode->GetInfo();
		if ((stmt.m_instruction == dThreeAdressStmt::m_assigment) && (stmt.m_operator == dThreeAdressStmt::m_nothing) && (stmt.m_arg1.m_type == dThreeAdressStmt::m_intConst)) {
			for (dCIL::dListNode* stmtNode1 = stmtNode->GetNext(); stmtNode1; stmtNode1 = stmtNode1->GetNext()) {
				dThreeAdressStmt& stmt1 = stmtNode1->GetInfo();
				switch (stmt1.m_instruction)
				{
					case dThreeAdressStmt::m_assigment:
					{
						if (stmt1.m_operator == dThreeAdressStmt::m_nothing) {
							if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
								ret = true;
								stmt1.m_arg1 = stmt.m_arg1;
								UpdateReachingDefinitions();
							}
						} else if (m_cil->m_commutativeOperator[stmt1.m_operator]) {
							
							if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
								ret = true;
								stmt1.m_arg1 = stmt1.m_arg2;
								stmt1.m_arg2 = stmt.m_arg1;
								if ((stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									stmt1.m_arg1 = stmt.m_arg1;
								}
								UpdateReachingDefinitions();
							} else if ((stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
								ret = true;
								stmt1.m_arg2 = stmt.m_arg1;
								UpdateReachingDefinitions();
							}
							
						} else {
							if ((stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
								ret = true;
								stmt1.m_arg2 = stmt.m_arg1;
								UpdateReachingDefinitions();
							}

							if ((stmt1.m_arg2.m_type == dThreeAdressStmt::m_intConst) && (stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
								ret = true;
								stmt1.m_arg1 = stmt.m_arg1;
								UpdateReachingDefinitions();
							}
						}

/*
						if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)){
							if ((stmt1.m_operator != dThreeAdressStmt::m_nothing) && m_cil->m_commutativeOperator[stmt1.m_operator]) {
							} else {
							ret = true;
							stmt1.m_arg1 = stmt.m_arg1;
						}
						if ((stmt1.m_operator != dThreeAdressStmt::m_nothing) && (stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
							ret = true;
							stmt1.m_arg2 = stmt.m_arg1;
						}
*/
						break;
					}

					case dThreeAdressStmt::m_if:
					{
						if ((stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && (stmt1.m_arg1.m_type == dThreeAdressStmt::m_intConst) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
							ret = true;
							stmt1.m_arg0 = stmt.m_arg1;
							UpdateReachingDefinitions();
						}

/*
						if ((stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
							ret = true;
							stmt1.m_arg0 = stmt.m_arg1;
						}
						if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
							ret = true;
							stmt1.m_arg1 = stmt.m_arg1;
						}
*/
						break;
					}
				}
			}
		}
	}

	return ret;
}


bool dDataFlowGraph::ApplyIfStatementsSimplification()
{
	bool ret = false;

	for (dCIL::dListNode* stmtNode = m_function; stmtNode; stmtNode = stmtNode->GetNext()) {
		dThreeAdressStmt& stmt = stmtNode->GetInfo();
		if ((stmt.m_instruction == dThreeAdressStmt::m_if) && (stmt.m_operator == dThreeAdressStmt::m_identical) || (stmt.m_operator == dThreeAdressStmt::m_different)) {
			dCIL::dListNode* const prevStmtNode = stmtNode->GetPrev();
			dThreeAdressStmt& stmtA = prevStmtNode->GetInfo();
			if ((stmt.m_arg1.m_label == stmtA.m_arg0.m_label) && (stmtA.m_instruction == dThreeAdressStmt::m_assigment)  &&  (stmtA.m_operator == dThreeAdressStmt::m_nothing) && (stmtA.m_arg1.m_type == dThreeAdressStmt::m_intConst)) {
				dCIL::dListNode* const prevPrevStmtNode = prevStmtNode->GetPrev();
				dThreeAdressStmt& stmtB = prevPrevStmtNode->GetInfo();
				if ((stmt.m_arg0.m_label == stmtB.m_arg0.m_label) && (stmtB.m_instruction == dThreeAdressStmt::m_assigment)  && (m_cil->m_conditionals[stmtB.m_operator]) && (stmtB.m_arg2.m_type != dThreeAdressStmt::m_intConst)) {
					stmt.m_arg0 = stmtB.m_arg1;
					stmt.m_arg1 = stmtB.m_arg2;
					if (stmt.m_operator == dThreeAdressStmt::m_identical) {
						stmt.m_operator = m_cil->m_operatorComplement[stmtB.m_operator];
					} else {
						stmt.m_operator = stmtB.m_operator;
					}
					ret = true;
				}
			}
		}
	}
	return ret;
}




bool dDataFlowGraph::ApplySubExpresionToCopyPropagation()
{
	bool ret = false;
	for (dCIL::dListNode* stmtNode = m_function; stmtNode; stmtNode = stmtNode->GetNext()) {
		dThreeAdressStmt& stmt = stmtNode->GetInfo();
		if (stmt.m_instruction == dThreeAdressStmt::m_assigment) {
			if ((stmt.m_operator != dThreeAdressStmt::m_nothing) && (stmt.m_arg0.m_label != stmt.m_arg1.m_label) && (stmt.m_arg0.m_label != stmt.m_arg2.m_label)){
				for (dCIL::dListNode* stmtNode1 = stmtNode->GetNext(); stmtNode1; stmtNode1 = stmtNode1->GetNext()) {
					dThreeAdressStmt& stmt1 = stmtNode1->GetInfo();
					switch (stmt1.m_instruction) 
					{
						case dThreeAdressStmt::m_assigment:
						{
							if ((stmt1.m_operator == dThreeAdressStmt::m_nothing) && (stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode) ){
								ret = true;
								stmt1.m_operator = stmt.m_operator;
								stmt1.m_arg1 = stmt.m_arg1;
								stmt1.m_arg2 = stmt.m_arg2;
								UpdateReachingDefinitions();
							}
							break;
						}

						case dThreeAdressStmt::m_if:
						{
							if (m_cil->m_conditionals[stmt.m_operator] && (stmt1.m_arg1.m_label == "0") && (stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
								switch (stmt1.m_operator) 
								{
									case dThreeAdressStmt::m_identical:
									{
										ret = true;
										stmt1.m_operator = m_cil->m_operatorComplement[stmt.m_operator];
										stmt1.m_arg0 = stmt.m_arg1;
										stmt1.m_arg1 = stmt.m_arg2;
										UpdateReachingDefinitions();
										break;
									}

									case dThreeAdressStmt::m_different:
									{
										ret = true;
										stmt1.m_operator = stmt.m_operator;
										stmt1.m_arg0 = stmt.m_arg1;
										stmt1.m_arg1 = stmt.m_arg2;
										UpdateReachingDefinitions();
										break;
									}
								}
							}
							break;
						}
					}
				}
			} 
		}
	}
	return ret;
}



bool dDataFlowGraph::ApplyCommonSubExpresion()
{
	bool ret = false;
	for (dCIL::dListNode* stmtNode = m_function; stmtNode; stmtNode = stmtNode->GetNext()) {
		dThreeAdressStmt& stmt = stmtNode->GetInfo();
		if ((stmt.m_instruction == dThreeAdressStmt::m_assigment) && (stmt.m_operator != dThreeAdressStmt::m_nothing)) {
			for (dCIL::dListNode* stmtNode1 = stmtNode->GetNext(); stmtNode1; stmtNode1 = stmtNode1->GetNext()) {
				dThreeAdressStmt& stmt1 = stmtNode1->GetInfo();
				if ((stmt1.m_instruction == dThreeAdressStmt::m_assigment) && (stmt1.m_operator == stmt.m_operator)) {
					if ((stmt.m_arg1.m_label == stmt1.m_arg1.m_label) && (stmt.m_arg2.m_label == stmt1.m_arg2.m_label)) {
						if (DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
							ret = true;
							stmt1.m_operator = stmt.m_nothing;
							stmt1.m_arg1 = stmt.m_arg0;
							stmt1.m_arg2.m_label = "";
							UpdateReachingDefinitions();
						}
					}
				}
			}
		}
	}

	return ret;
}


bool dDataFlowGraph::ApplyConstantFolding()
{
	bool ret = false;
	for (dCIL::dListNode* stmtNode = m_function; stmtNode; stmtNode = stmtNode->GetNext()) {
		dThreeAdressStmt& stmt = stmtNode->GetInfo();

		switch  (stmt.m_instruction) 
		{
			case dThreeAdressStmt::m_if:
			{
				if (stmt.m_arg1.m_type == dThreeAdressStmt::m_intConst) {
					ret = true;
					int val = EvaluateBinaryExpression (stmt.m_arg0.m_label, stmt.m_operator, stmt.m_arg1.m_label);
					if (val) {
						stmt.m_instruction = dThreeAdressStmt::m_goto;
						stmt.m_arg0.m_label = stmt.m_arg2.m_label;
					} else {
						stmt.m_instruction = dThreeAdressStmt::m_nop;
					}
				} else if ((stmt.m_arg0.m_type == dThreeAdressStmt::m_intVar) && (stmt.m_arg1.m_type == dThreeAdressStmt::m_intVar)) {
					dList<dCIL::dListNode*>& argListNodeA = m_variableDefinitions.Find (stmt.m_arg0.m_label)->GetInfo();
					dList<dCIL::dListNode*>& argListNodeB = m_variableDefinitions.Find (stmt.m_arg1.m_label)->GetInfo();
					if ((argListNodeA.GetCount() == 1) && (argListNodeB.GetCount() == 1)) {
						dThreeAdressStmt& argStmtA = argListNodeA.GetFirst()->GetInfo()->GetInfo();
						dThreeAdressStmt& argStmtB = argListNodeB.GetFirst()->GetInfo()->GetInfo();
						if ((argStmtA.m_arg1.m_type == dThreeAdressStmt::m_intConst) && (argStmtB.m_arg1.m_type == dThreeAdressStmt::m_intConst)) {
							ret = true;
							stmt.m_arg0 = argStmtA.m_arg1;
							stmt.m_arg1 = argStmtB.m_arg1;
							int val = EvaluateBinaryExpression (stmt.m_arg0.m_label, stmt.m_operator, stmt.m_arg1.m_label);
							if (val) {
								stmt.m_instruction = dThreeAdressStmt::m_goto;
								stmt.m_arg0.m_label = stmt.m_arg2.m_label;
							} else {
								stmt.m_instruction = dThreeAdressStmt::m_nop;
							}
							UpdateReachingDefinitions();
//m_cil->Trace();
						}
					}
				}
				break;
			}

			case dThreeAdressStmt::m_assigment:
			{
				if (stmt.m_operator != dThreeAdressStmt::m_nothing) {
					if (((stmt.m_arg1.m_type == dThreeAdressStmt::m_intConst) || (stmt.m_arg1.m_type == dThreeAdressStmt::m_floatConst)) &&
						((stmt.m_arg2.m_type == dThreeAdressStmt::m_intConst) || (stmt.m_arg2.m_type == dThreeAdressStmt::m_floatConst))) {
						ret = true;
						if ((stmt.m_arg1.m_type == dThreeAdressStmt::m_intConst) && (stmt.m_arg1.m_type == dThreeAdressStmt::m_intConst)) {
							int arg0 = EvaluateBinaryExpression (stmt.m_arg1.m_label, stmt.m_operator, stmt.m_arg2.m_label);
							stmt.m_operator = dThreeAdressStmt::m_nothing;
							stmt.m_arg1.m_label = dString(arg0);
							stmt.m_arg2.m_label = dString ("");

						} else if ((stmt.m_arg1.m_type == dThreeAdressStmt::m_floatConst) && (stmt.m_arg1.m_type == dThreeAdressStmt::m_floatConst)) {
							dAssert (0);
						} else {
							dAssert (0);
						}
					}
				}
				break;
			}
		}
	}
	return ret;
}





void dDataFlowGraph::GetLoops (dList<dLoop>& loops) const
{

	for (dCIL::dListNode* node = m_function; node; node = node->GetNext()) {
		dThreeAdressStmt& stmt = node->GetInfo();	
		if ((stmt.m_instruction == dThreeAdressStmt::m_nop) && (stmt.m_arg2.m_label == D_LOOP_HEADER_SYMBOL)) {
			dList<dLoop>::dListNode* const loopNode = loops.Addtop();
			dLoop& loop = loopNode->GetInfo();
			loop.m_head = node;

			int order = stmt.m_extraInformation;
			for (dList<dLoop>::dListNode* root = loops.GetLast(); root != loops.GetFirst(); root = root->GetPrev()) {
				int code = root->GetInfo().m_head->GetInfo().m_extraInformation;
				if (code > order) {
					loops.InsertAfter (root, loopNode);
					break;
				}
			}

			for (dCIL::dListNode* node1 = node->GetNext(); node1; node1 = node1->GetNext()) {
				dThreeAdressStmt& stmt = node1->GetInfo();
				int code = stmt.m_extraInformation;
				if ((stmt.m_instruction == dThreeAdressStmt::m_nop) && (code == order) && (stmt.m_arg2.m_label == D_LOOP_TAIL_SYMBOL)) {
					loop.m_tail = node1;
					break;
				}
			}
		}
	}
}


bool dDataFlowGraph::IsStatementInReachList(dCIL::dListNode* const node, dList<dCIL::dListNode*>& definitionList, dCIL::dListNode* const me) const
{
	const dDataFlowPoint& point = m_dataFlowGraph.Find (node)->GetInfo();
	for (dList<dCIL::dListNode*>::dListNode* ptr = definitionList.GetFirst(); ptr; ptr = ptr->GetNext()) {
		dCIL::dListNode* const defVarStmt = ptr->GetInfo();
		if (defVarStmt != me) {
			if (point.m_reachStmtInputSet.Find(defVarStmt)) {
				return true;
			}
		}
	}
	return false;
}

bool dDataFlowGraph::CheckBackEdgePreReachInLoop(dCIL::dListNode* const stmtNode, const dLoop& loop) const
{
	bool ret = false;
	const dString& var = stmtNode->GetInfo().m_arg0.m_label;
	dList<dCIL::dListNode*>& statementList = m_variableDefinitions.Find (var)->GetInfo();

	for (dCIL::dListNode* node = loop.m_head; !ret && (node != loop.m_tail->GetPrev()); node = node->GetNext()) {
		dThreeAdressStmt& stmt = node->GetInfo();
		switch (stmt.m_instruction)
		{
			case dThreeAdressStmt::m_if:
			case dThreeAdressStmt::m_assigment:
			{
				if ((stmt.m_arg1.m_label == var) || (stmt.m_arg2.m_label == var)) {
					ret = IsStatementInReachList(node, statementList, stmtNode);
				}
				break;
			}

			case dThreeAdressStmt::m_store:
			{
				if ((stmt.m_arg0.m_label == var) || (stmt.m_arg1.m_label == var) || (stmt.m_arg2.m_label == var)) {
					ret = IsStatementInReachList(node, statementList, stmtNode);
				}
				break;
			}
		

			case dThreeAdressStmt::m_nop:
			case dThreeAdressStmt::m_label:
				break;

			default:
				dAssert (0);
		}
	}
	return ret;
}


bool dDataFlowGraph::ApplyLoopOptimization(dLoop& loop)
{
	bool ret = false;

	dAssert (0);
/*
	// calculate the dominator tree for this loop
	dTree <dDominator, dCIL::dListNode*> dominatorTree;
	dDominator& rootDominator = dominatorTree.Insert (loop.m_head)->GetInfo(); 
	rootDominator.Insert (0, loop.m_head);
	for (dCIL::dListNode* node = loop.m_head->GetNext(); node != loop.m_tail; node = node->GetNext()) {
		dDominator& dominator = dominatorTree.Insert (node)->GetInfo(); 
		for (dCIL::dListNode* node1 = loop.m_head; node1 != loop.m_tail; node1 = node1->GetNext()) {
			dominator.Insert (0, node1);
		}
	}

	bool someSetChanged = false;
	while (!someSetChanged) {
		someSetChanged = true;

		for (dCIL::dListNode* node = loop.m_head->GetNext(); node != loop.m_tail; node = node->GetNext()) {
			dAssert (m_dataFlowGraph.Find(node));
			dDominator& dominator = dominatorTree.Find (node)->GetInfo(); 
			dDominator oldDominator (dominator);

			dDataFlowPoint& info = m_dataFlowGraph.Find(node)->GetInfo();
			for (dList<dDataFlowPoint*>::dListNode* predecessorsNode = info.m_predecessors.GetFirst(); predecessorsNode; predecessorsNode = predecessorsNode->GetNext()) {

				dDataFlowPoint* const predecessorsInfo = predecessorsNode->GetInfo();
				dCIL::dListNode* const stmtNode = predecessorsInfo->m_statement;
				dDominator& parentDominator = dominatorTree.Find (stmtNode)->GetInfo(); 
				dominator.Intersection (parentDominator);
			}
			dominator.Insert (node);

			someSetChanged = (someSetChanged && dominator.Compare(oldDominator));
		}
	}


#ifdef _DEBUG
	for (dCIL::dListNode* node = loop.m_head; node != loop.m_tail; node = node->GetNext()) {
		dThreeAdressStmt& stmt = node->GetInfo();	
		dTrace (("%d ", stmt.m_debug));
		DTRACE_INTRUCTION(&stmt);
	}

	dTrace (("\n\n"));

	for (dCIL::dListNode* node = loop.m_head; node != loop.m_tail; node = node->GetNext()) {
		dDominator& dominator = dominatorTree.Find (node)->GetInfo(); 
		dThreeAdressStmt& stmt = node->GetInfo();	
		dTrace (("%d: ", stmt.m_debug));

		dDominator::Iterator iter (dominator);
		for (iter.Begin(); iter; iter ++) {
			dCIL::dListNode* const dominatedNode = iter.GetKey();
			dThreeAdressStmt& stmt = dominatedNode->GetInfo();	
			dTrace (("%d ", stmt.m_debug));
		}
		dTrace (("\n"));
	}
#endif

	
	// mark all potential loop invariant candidates for code promotion	
	dTree <dDominator, dCIL::dListNode*>::Iterator iter (dominatorTree);
	for (iter.Begin(); iter; iter ++) {
		dDominator& dominator = iter.GetNode()->GetInfo();
		dThreeAdressStmt& stmt = iter.GetKey()->GetInfo();

		switch (stmt.m_instruction) 
		{
			case dThreeAdressStmt::m_assigment:
			{
				if (stmt.m_operator == dThreeAdressStmt::m_nothing) {	
					if ((stmt.m_arg1.m_type == dThreeAdressStmt::m_intConst) || (stmt.m_arg1.m_type == dThreeAdressStmt::m_floatConst)) {
						dominator.m_isLoopInvariant = true;
					} else {
						dAssert (0);
					}
				} else {
					//dAssert (0);
				}
				break;
			}

			case dThreeAdressStmt::m_paramLoad:
			{
				dominator.m_isLoopInvariant = true;
				break;
			}
		}
	}

	CalculateLiveInputLiveOutput();
	UpdateReachingDefinitions();

	dDataFlowPoint& liveInfoAtTail = m_dataFlowGraph.Find(loop.m_tail)->GetInfo();
	dDataFlowPoint::dVariableSet<dString>& liveOut = liveInfoAtTail.m_liveOutputSet;


	dDominator& loopExitDominators = dominatorTree.Find (loop.m_tail->GetPrev()->GetPrev())->GetInfo(); 
	dCIL::dListNode* preHeaderNode = loop.m_head->GetPrev();
	dCIL::dListNode* nextNode;
	for (dCIL::dListNode* node = loop.m_head; node != loop.m_tail->GetPrev(); node = nextNode) {
		nextNode = node->GetNext();

		dDominator& dominator = dominatorTree.Find (node)->GetInfo(); 
		if (dominator.m_isLoopInvariant) {
			dThreeAdressStmt& stmt = node->GetInfo();
			switch (stmt.m_instruction) 
			{
				case dThreeAdressStmt::m_assigment:
				{
					if (stmt.m_operator == dThreeAdressStmt::m_nothing) {
						if (stmt.m_arg1.m_type == dThreeAdressStmt::m_intConst) {
							if (loopExitDominators.Find (node) || !liveOut.Find (stmt.m_arg0.m_label)) {
								dAssert (m_variableDefinitions.Find (stmt.m_arg0.m_label));
								dList<dCIL::dListNode*>& varDefintionList = m_variableDefinitions.Find (stmt.m_arg0.m_label)->GetInfo();
								if ((varDefintionList.GetCount() == 1) || !CheckBackEdgePreReachInLoop(node, loop)) {
									ret = 1;
									dCIL::dListNode* const movedNode = m_cil->AppendAfter (preHeaderNode);
									movedNode->GetInfo() = stmt;
									stmt.m_instruction = dThreeAdressStmt::m_nop;
									stmt.m_arg0.m_label = "";
									stmt.m_arg1.m_label = "";
									stmt.m_arg2.m_label = "";
									stmt.m_extraInformation = 0;

									preHeaderNode = movedNode;
									loop.m_head = loop.m_head->GetNext();
								}
							}
						}
					}
					break;
				}

				case dThreeAdressStmt::m_paramLoad:
				{
					if (loopExitDominators.Find (node) || !liveOut.Find (stmt.m_arg0.m_label)) {
						dAssert (m_variableDefinitions.Find (stmt.m_arg0.m_label));
						dList<dCIL::dListNode*>& varDefintionList = m_variableDefinitions.Find (stmt.m_arg0.m_label)->GetInfo();
						if ((varDefintionList.GetCount() == 1) || !CheckBackEdgePreReachInLoop(node, loop)) {
							if (!CheckBackEdgePreReachInLoop(node, loop)) {
								ret = 1;
								dCIL::dListNode* const movedNode = m_cil->AppendAfter (preHeaderNode);
								movedNode->GetInfo() = stmt;
								stmt.m_instruction = dThreeAdressStmt::m_nop;
								stmt.m_arg0.m_label = "";
								stmt.m_arg1.m_label = "";
								stmt.m_arg2.m_label = "";
								stmt.m_extraInformation = 0;

								preHeaderNode = movedNode;
								loop.m_head = loop.m_head->GetNext();
							}
						}
					}
					break;
				}
			}
		}
	}
*/
	return ret;
}

void dDataFlowGraph::ApplyLocalOptimizations()
{
	m_mark += 2;
	BuildBasicBlockGraph();
//m_cil->Trace();

	ApplyIfStatementsSimplification();
//m_cil->Trace();

	CalculateReachingDefinitions();
	for (bool optimized = true; optimized;) {
		optimized = false;
		optimized |= ApplySubExpresionToCopyPropagation();
//m_cil->Trace();
//m_basicBlocks.Trace();
		if (optimized) {
			ApplyRemoveDeadCode();
//m_cil->Trace();
//m_basicBlocks.Trace();
		}
	}

	// apply generic data flow optimization for single blocks
	for (bool optimized = true; optimized;) {
		optimized = false;
		UpdateReachingDefinitions();

		optimized |= ApplyConstantPropagation();
//m_cil->Trace();
//m_basicBlocks.Trace();
		optimized |= ApplyCopyPropagation();
//m_cil->Trace();
//m_basicBlocks.Trace();
		optimized |= ApplyConstantFolding();
//m_cil->Trace();
//m_basicBlocks.Trace();
		optimized |= ApplyInstructionSematicOrdering();
//m_cil->Trace();
//m_basicBlocks.Trace();
		optimized |= ApplyRemoveDeadCode();
//m_cil->Trace();
//m_basicBlocks.Trace();
	}

	// apply peephole optimizations
	for (bool optimized = true; optimized;) {
		optimized = false;
		UpdateReachingDefinitions();

		optimized |= ApplySubExpresionToCopyPropagation();
//m_cil->Trace();
//m_basicBlocks.Trace();
		if (optimized) {
			ApplyRemoveDeadCode();
//m_cil->Trace();
//m_basicBlocks.Trace();
		}
	}

	for (bool optimized = true; optimized;) {
		optimized = false;
		UpdateReachingDefinitions();

		optimized |= ApplyCommonSubExpresion();
//m_cil->Trace();
//m_basicBlocks.Trace();
		optimized |= ApplyCopyPropagation();
//m_cil->Trace();
//m_basicBlocks.Trace();
		optimized |= ApplyRemoveDeadCode();
//m_cil->Trace();
//m_basicBlocks.Trace();
	}

/*
	// apply loops optimizations here
	dList<dLoop> loops;
	GetLoops (loops);
	for (bool optimized = true; optimized;) {
		optimized = false;
		for (dList<dLoop>::dListNode* loopNode = loops.GetFirst(); loopNode; loopNode = loopNode->GetNext()) {
			dLoop& loop = loopNode->GetInfo();
			optimized |= ApplyLoopOptimization(loop);
m_cil->Trace();
		}
	}
*/

}
#endif


void dDataFlowGraph::BuildDefinedAndKilledStatementSets()
{
	dTree<dDataFlowPoint, dCIL::dListNode*>::Iterator iter (m_dataFlowGraph);
	for (iter.Begin(); iter; iter ++) {
		dDataFlowPoint& point = iter.GetNode()->GetInfo();
		point.m_killStmtSet.RemoveAll();
	}

	m_variableDefinitions.RemoveAll();
//	dDataFlowPoint::dVariableSet<dCIL::dListNode*> statementUsingReturnVariable;
	for (dCIL::dListNode* ptr = m_basicBlocks.m_begin; ptr != m_basicBlocks.m_end; ptr = ptr->GetNext()) {
		dCILInstr* const instruc = ptr->GetInfo();
//instruc->Trace();
		instruc->AddDefinedVariable (m_variableDefinitions);

/*

		dThreeAdressStmt& stmt = ptr->GetInfo();	

//DTRACE_INTRUCTION (&stmt);		
		switch (stmt.m_instruction)
		{
			case dThreeAdressStmt::m_argument:
			case dThreeAdressStmt::m_assigment:
			{
				dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const node = m_variableDefinitions.Insert(stmt.m_arg0.m_label);
				node->GetInfo().Append(ptr);
				break;
			}

			case dThreeAdressStmt::m_loadBase:
			{
				dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const node = m_variableDefinitions.Insert(stmt.m_arg0.m_label);
				node->GetInfo().Append(ptr);
				break;
			}


#if 0
			case dThreeAdressStmt::m_load:
			{
				dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const node = m_variableDefinitions.Insert(stmt.m_arg0.m_label);
				node->GetInfo().Append(ptr);

				if (stmt.m_arg1.m_label == m_returnVariableName) {
					statementUsingReturnVariable.Insert(ptr);
				}

				if (stmt.m_arg2.m_label == m_returnVariableName) {
					statementUsingReturnVariable.Insert(ptr);
				}
				break;
			}
            
			case dThreeAdressStmt::m_store:
			{
				if (stmt.m_arg0.m_label == m_returnVariableName) {
					statementUsingReturnVariable.Insert(ptr);
				}
				break;
			}

            case dThreeAdressStmt::m_new:
            {
                dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const node = m_variableDefinitions.Insert(stmt.m_arg0.m_label);
                node->GetInfo().Append(ptr);

                if (stmt.m_arg1.m_label == m_returnVariableName) {
                    statementUsingReturnVariable.Insert(ptr);
                }
                break;
             }

            case dThreeAdressStmt::m_release:
            {
                if (stmt.m_arg0.m_label == m_returnVariableName) {
                    statementUsingReturnVariable.Insert(ptr);
                }
                break;
            }


			case dThreeAdressStmt::m_push:
			{
				if (stmt.m_arg1.m_label == m_returnVariableName) {
					statementUsingReturnVariable.Insert(ptr);
				}
				break;
			}

			case dThreeAdressStmt::m_if:
			{
				if (stmt.m_arg0.m_type == dThreeAdressStmt::m_intVar) {
					if (stmt.m_arg0.m_label == m_returnVariableName) {
						statementUsingReturnVariable.Insert(ptr);
					}
				}

				if (stmt.m_arg1.m_type == dThreeAdressStmt::m_intVar) {
					if (stmt.m_arg1.m_label == m_returnVariableName) {
						statementUsingReturnVariable.Insert(ptr);
					}
				}
				break;
			}

			
			{
				dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const node = m_variableDefinitions.Insert(stmt.m_arg2.m_label);
				node->GetInfo().Append(ptr);

//				if (stmt.m_arg0.m_label == m_returnVariableName) {
//					statementUsingReturnVariable.Insert(ptr);
//				}
//				break;
			}

#endif

			case dThreeAdressStmt::m_call:
			{
				if (stmt.m_arg0.GetType().m_isPointer || (stmt.m_arg0.GetType().m_intrinsicType != dThreeAdressStmt::m_void)) {
					dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const node = m_variableDefinitions.Insert(stmt.m_arg0.m_label);
					node->GetInfo().Append(ptr);
				}
				break;					
			}

			//case dThreeAdressStmt::m_pop:
			//case dThreeAdressStmt::m_enter:
			//case dThreeAdressStmt::m_leave:
			case dThreeAdressStmt::m_param:
			case dThreeAdressStmt::m_if:
			case dThreeAdressStmt::m_ifnot:
			case dThreeAdressStmt::m_ret:
			case dThreeAdressStmt::m_goto:
			case dThreeAdressStmt::m_nop:
			case dThreeAdressStmt::m_label:
			case dThreeAdressStmt::m_function:
			case dThreeAdressStmt::m_storeBase:
				break;

			default:
				dAssert (0);
		}
*/
	}

	for (iter.Begin(); iter; iter ++) {
		dDataFlowPoint& point = iter.GetNode()->GetInfo();

		point.m_generateStmt = false;
		//dThreeAdressStmt& stmt = point.m_statement->GetInfo();	
		dCILInstr* const instruc = point.m_statement->GetInfo();
//instruc->Trace();
		instruc->AddKilledStatements(m_variableDefinitions, point);
		
/*
//DTRACE_INTRUCTION (&stmt);		
		switch (stmt.m_instruction)
		{
			case dThreeAdressStmt::m_param:
			{
				point.m_generateStmt = true;
				dAssert (m_variableDefinitions.Find(stmt.m_arg0.m_label));
				dList<dCIL::dListNode*>& defsList = m_variableDefinitions.Find(stmt.m_arg0.m_label)->GetInfo();
				for (dList<dCIL::dListNode*>::dListNode* defNode = defsList.GetFirst(); defNode; defNode = defNode->GetNext()) {
					dCIL::dListNode* const killStement = defNode->GetInfo();
					if (killStement != point.m_statement) {
						point.m_killStmtSet.Insert(killStement);
					}
				}
				break;
			}

//			case dThreeAdressStmt::m_storeBase:
//			case dThreeAdressStmt::m_loadBase:
//			{
//				point.m_generateStmt = true;
//				dAssert (m_variableDefinitions.Find(stmt.m_arg2.m_label));
//				dList<dCIL::dListNode*>& defsList = m_variableDefinitions.Find(stmt.m_arg2.m_label)->GetInfo();
//				for (dList<dCIL::dListNode*>::dListNode* defNode = defsList.GetFirst(); defNode; defNode = defNode->GetNext()) {
//					dCIL::dListNode* const killStement = defNode->GetInfo();
//					if (killStement != point.m_statement) {
//						point.m_killStmtSet.Insert(killStement);
//					}
//				}
//				break;
//			}
//			case dThreeAdressStmt::m_param:
//			{
//				point.m_generateStmt = true;
//				dAssert (m_variableDefinitions.Find(stmt.m_arg0.m_label));
//				dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const retRegisterNode = m_variableDefinitions.Find(stmt.m_arg0.m_label);
//				dList<dCIL::dListNode*>& defsList = retRegisterNode->GetInfo();
//				for (dList<dCIL::dListNode*>::dListNode* defNode = defsList.GetFirst(); defNode; defNode = defNode->GetNext()) {
//					dCIL::dListNode* const killStement = defNode->GetInfo();
//					if (killStement != point.m_statement) {
//						point.m_killStmtSet.Insert(killStement);
//					}
//				}
//				break;
//			}


			//case dThreeAdressStmt::m_enter:
			//case dThreeAdressStmt::m_leave:
			//case dThreeAdressStmt::m_load:
			case dThreeAdressStmt::m_store:
			case dThreeAdressStmt::m_storeBase:
			//case dThreeAdressStmt::m_push:
			//case dThreeAdressStmt::m_pop:
			case dThreeAdressStmt::m_function:
			case dThreeAdressStmt::m_nop:
			case dThreeAdressStmt::m_if:
			case dThreeAdressStmt::m_ifnot:
			case dThreeAdressStmt::m_ret:
			case dThreeAdressStmt::m_goto:
			case dThreeAdressStmt::m_label:
			case dThreeAdressStmt::m_argument:
				break;

			default:
				dAssert (0);
		}
*/
	}
}



void dDataFlowGraph::CalculateReachingDefinitions()
{
	BuildDefinedAndKilledStatementSets ();

	dTree<dDataFlowPoint, dCIL::dListNode*>::Iterator iter (m_dataFlowGraph);
	for (iter.Begin(); iter; iter ++) {
		dDataFlowPoint& point = iter.GetNode()->GetInfo();
		point.m_reachStmtInputSet.RemoveAll();
		point.m_reachStmtOutputSet.RemoveAll();
	}

	bool someSetChanged = false;
	while (!someSetChanged) {
		someSetChanged = true;
		for (dList<dBasicBlock*>::dListNode* blockNode = m_traversalBlocksOrder.GetLast(); blockNode; blockNode = blockNode->GetPrev()) {
			dBasicBlock* const block = blockNode->GetInfo();

			int test = 1;
			for (dCIL::dListNode* stmtNode = block->m_begin; test; stmtNode = stmtNode->GetNext()) {
				test = (stmtNode != block->m_end);
				dAssert (m_dataFlowGraph.Find(stmtNode));
				dDataFlowPoint& info = m_dataFlowGraph.Find(stmtNode)->GetInfo();

				//DTRACE_INTRUCTION (&info.m_statement->GetInfo());		

				dDataFlowPoint::dVariableSet<dCIL::dListNode*> oldInput (info.m_reachStmtInputSet);
				dDataFlowPoint::dVariableSet<dCIL::dListNode*> oldOutput (info.m_reachStmtOutputSet);

				info.m_reachStmtInputSet.RemoveAll();
				for (dList<dDataFlowPoint*>::dListNode* predecessorsNode = info.m_predecessors.GetFirst(); predecessorsNode; predecessorsNode = predecessorsNode->GetNext()) {
					dDataFlowPoint* const predecessorsInfo = predecessorsNode->GetInfo();
					info.m_reachStmtInputSet.Union (predecessorsInfo->m_reachStmtOutputSet);
				}

				info.m_reachStmtOutputSet.RemoveAll();
				info.m_reachStmtOutputSet.Difference (info.m_reachStmtInputSet, info.m_killStmtSet);
				//info.m_reachOutputSet.Union (info.m_generateVariable);
				if (info.m_generateStmt) {
					info.m_reachStmtOutputSet.Insert (info.m_statement);
				}

				someSetChanged = (someSetChanged && oldOutput.Compare(info.m_reachStmtOutputSet) && oldInput.Compare(info.m_reachStmtInputSet));
			}
		}
	}


//#ifdef _DEBUG
#if 0
	for (dCIL::dListNode* ptr = m_function; ptr; ptr = ptr->GetNext()) {
		dThreeAdressStmt& stmt = ptr->GetInfo();	
		if ((stmt.m_instruction != dThreeAdressStmt::m_nop) && (stmt.m_instruction != dThreeAdressStmt::m_function)) {
			dTrace (("%d ", stmt.m_debug));
			DTRACE_INTRUCTION(&stmt);
		}
	}

	dTrace (("\n\n"));

	for (dCIL::dListNode* ptr = m_function; ptr; ptr = ptr->GetNext()) {
		dThreeAdressStmt& stmt = ptr->GetInfo();	
		if ((stmt.m_instruction != dThreeAdressStmt::m_nop) && (stmt.m_instruction != dThreeAdressStmt::m_function)) {
			dTrace (("%d ", stmt.m_debug));
			DTRACE_INTRUCTION(&stmt);
			const dDataFlowPoint& point = m_dataFlowGraph.Find(ptr)->GetInfo() ;

			if (point.m_reachStmtInputSet.GetCount()) {

				dTrace (("\t\t"));
				dDataFlowPoint::dVariableSet<dCIL::dListNode*>::Iterator iter(point.m_reachStmtInputSet);
				for (iter.Begin(); iter; iter ++) {
					dCIL::dListNode* const stmtNode = iter.GetKey();
					dThreeAdressStmt& stmt1 = stmtNode->GetInfo();
					dTrace (("%d ", stmt1.m_debug));
				}
				dTrace (("\n"));
			}
		}
	}

#endif

}

void dDataFlowGraph::UpdateLiveInputLiveOutput()
{
	CalculateLiveInputLiveOutput ();
}

void dDataFlowGraph::UpdateReachingDefinitions()
{
	// for now just brute force regeneration
	CalculateReachingDefinitions();
}


void dDataFlowGraph::BuildGeneratedAndUsedVariableSets()
{
	dTree<dDataFlowPoint, dCIL::dListNode*>::Iterator iter (m_dataFlowGraph);
	for (iter.Begin(); iter; iter ++) {
		dDataFlowPoint& point = iter.GetNode()->GetInfo();
		point.m_usedVariableSet.RemoveAll();
	}

	for (iter.Begin(); iter; iter ++) {
		dDataFlowPoint& point = iter.GetNode()->GetInfo();
		point.m_generatedVariable.Empty();
		dCILInstr* const instruc = point.m_statement->GetInfo();
//instruc->Trace();
		instruc->AddGeneratedAndUsedSymbols(point);	

/*
		dThreeAdressStmt& stmt = point.m_statement->GetInfo();	
//DTRACE_INTRUCTION (&stmt);		

		point.m_generatedVariable.Empty();
		switch (stmt.m_instruction)
		{
			case dThreeAdressStmt::m_argument:
			{
				point.m_generatedVariable = stmt.m_arg0.m_label;
				break;
			}

			case dThreeAdressStmt::m_load:
			{
				dAssert (0);
				point.m_generatedVariable = stmt.m_arg0.m_label;
				point.m_usedVariableSet.Insert(stmt.m_arg1.m_label);
				point.m_usedVariableSet.Insert(stmt.m_arg2.m_label);
				break;
			}

			case dThreeAdressStmt::m_store:
			{
				dAssert (0);
				point.m_usedVariableSet.Insert(stmt.m_arg0.m_label);
				point.m_usedVariableSet.Insert(stmt.m_arg1.m_label);
				point.m_usedVariableSet.Insert(stmt.m_arg2.m_label);
				break;
			}


			case dThreeAdressStmt::m_new:
			{
				point.m_generatedVariable = stmt.m_arg0.m_label;
				point.m_usedVariableSet.Insert(stmt.m_arg1.m_label);
				break;
			}

			case dThreeAdressStmt::m_release:
			{
				point.m_usedVariableSet.Insert(stmt.m_arg0.m_label);
				break;
			}

			case dThreeAdressStmt::m_ifnot:
			{
				if (stmt.m_arg0.GetType().m_intrinsicType == dThreeAdressStmt::m_int) {
					point.m_usedVariableSet.Insert(stmt.m_arg0.m_label);
				} else {
					dAssert (0);
				}
				break;
			}

//			case dThreeAdressStmt::m_push:
//			{
//				point.m_usedVariableSet.Insert(stmt.m_arg0.m_label);
//				break;
//			}


//			case dThreeAdressStmt::m_pop:
			case dThreeAdressStmt::m_nop:
			case dThreeAdressStmt::m_goto:
			case dThreeAdressStmt::m_label:
			//case dThreeAdressStmt::m_enter:
			//case dThreeAdressStmt::m_leave:
			case dThreeAdressStmt::m_function:
				break;

			default:
				dAssert (0);
		}
*/
	}
}


void dDataFlowGraph::CalculateLiveInputLiveOutput ()
{
	BuildGeneratedAndUsedVariableSets();

	dTree<dDataFlowPoint, dCIL::dListNode*>::Iterator iter (m_dataFlowGraph);
	for (iter.Begin(); iter; iter ++) {
		dDataFlowPoint& point = iter.GetNode()->GetInfo();
		point.m_liveInputSet.RemoveAll();
		point.m_liveOutputSet.RemoveAll();
	}

	bool someSetChanged = false;
	while (!someSetChanged) {
		someSetChanged = true;
		for (dList<dBasicBlock*>::dListNode* blockNode = m_traversalBlocksOrder.GetFirst(); blockNode; blockNode = blockNode->GetNext()) {

			dBasicBlock* const block = blockNode->GetInfo();

			int test = 1;
			for (dCIL::dListNode* node = block->m_end; test; node = node->GetPrev()) {
				test = (node != block->m_begin);

				dAssert (m_dataFlowGraph.Find(node));
				dDataFlowPoint& info = m_dataFlowGraph.Find(node)->GetInfo();
				dDataFlowPoint::dVariableSet<dString> oldInput (info.m_liveInputSet);
				dDataFlowPoint::dVariableSet<dString> oldOutput (info.m_liveOutputSet);

				info.m_liveInputSet.RemoveAll();
				info.m_liveInputSet.Union(info.m_liveOutputSet);
				if (info.m_generatedVariable.Size()) {
					info.m_liveInputSet.Remove(info.m_generatedVariable);
				}
				info.m_liveInputSet.Union(info.m_usedVariableSet);

				info.m_liveOutputSet.RemoveAll();
				for (dList<dDataFlowPoint*>::dListNode* successorNode = info.m_successors.GetFirst(); successorNode; successorNode = successorNode->GetNext()) {
					dDataFlowPoint* const successorInfo = successorNode->GetInfo();
					info.m_liveOutputSet.Union (successorInfo->m_liveInputSet);
				}
				someSetChanged = (someSetChanged && oldOutput.Compare(info.m_liveOutputSet) && oldInput.Compare(info.m_liveInputSet));
			}
		}
	}

#if 0
	for (dCIL::dListNode* node = m_basicBlocks.m_begin; node != m_basicBlocks.m_end; node = node->GetNext()) {
		dTree<dDataFlowPoint, dCIL::dListNode*>::dTreeNode* const pointNode = m_dataFlowGraph.Find(node);
		if (pointNode) {
			dDataFlowPoint& info = pointNode->GetInfo();
			node->GetInfo()->Trace();

			dDataFlowPoint::dVariableSet<dString>::Iterator iter(info.m_liveOutputSet);
			if (info.m_liveOutputSet.GetCount()) {
				dTrace(("           :"));
				info.m_liveOutputSet.Trace();
				//for (iter.Begin(); iter; iter++) {
				//				dTrace(("%s ", iter.GetKey().GetStr()));
				//}
				//dTrace(("\n"));
			}
		}
	}
#endif
}

void dDataFlowGraph::FindNodesInPathway(dCIL::dListNode* const source, dCIL::dListNode* const destination, dTree<int, dCIL::dListNode*>& pathOut) const
{
	m_mark ++;
	dList<dDataFlowPoint*> queue; 
	queue.Append(&m_dataFlowGraph.Find(source)->GetInfo());
	dAssert (queue.GetFirst()->GetInfo()->m_statement == source);

	while (queue.GetCount()) {
		dDataFlowPoint* const rootNode = queue.GetFirst()->GetInfo();
		queue.Remove(queue.GetFirst());
		if (rootNode->m_mark != m_mark) {
			rootNode->m_mark = m_mark;
			pathOut.Insert(rootNode->m_statement);

			for (dList<dDataFlowPoint*>::dListNode* successorsNode = rootNode->m_successors.GetFirst(); successorsNode; successorsNode = successorsNode->GetNext()) {
				dDataFlowPoint* const successor = successorsNode->GetInfo();
				if (successor->m_statement != destination) {
					queue.Append(successor);
				}
			}
		}
	}
//	pathOut.Remove(source);
}


bool dDataFlowGraph::DoStatementAreachesStatementB(dCIL::dListNode* const stmtNodeB, dCIL::dListNode* const stmtNodeA) const
{
	bool canApplyPropagation = false;	
/*
	dDataFlowPoint& info = m_dataFlowGraph.Find(stmtNodeB)->GetInfo();
	dDataFlowPoint::dVariableSet<dCIL::dListNode*>& reachingInputs = info.m_reachStmtInputSet;

	if (reachingInputs.Find (stmtNodeA)) {
		canApplyPropagation = true;

		const dThreeAdressStmt& constStmt = stmtNodeA->GetInfo();
		dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const definitions = m_variableDefinitions.Find(constStmt.m_arg0.m_label);
		dAssert (definitions);

		dList<dCIL::dListNode*>& defintionList = definitions->GetInfo();
		for (dList<dCIL::dListNode*>::dListNode* otherStmtNode = defintionList.GetFirst(); otherStmtNode; otherStmtNode = otherStmtNode->GetNext()){
			dCIL::dListNode* const duplicateDefinition = otherStmtNode->GetInfo();
			if (duplicateDefinition != stmtNodeA) {
				if (reachingInputs.Find(duplicateDefinition)) {
					if (stmtNodeB->GetInfo().m_instruction != dThreeAdressStmt::m_loadBase) {
						return false;
					}
				}
			}
		}
		
        bool isSpecialStoreType = (constStmt.m_instruction == dThreeAdressStmt::m_storeBase);
		if (isSpecialStoreType || (constStmt.m_arg1.GetType().m_intrinsicType == dThreeAdressStmt::m_int) || (constStmt.m_arg1.GetType().m_intrinsicType == dThreeAdressStmt::m_float)) {
			dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const definedStatements = isSpecialStoreType ? m_variableDefinitions.Find(constStmt.m_arg2.m_label) : m_variableDefinitions.Find(constStmt.m_arg1.m_label);
			if (definedStatements) {
				dTree<int, dCIL::dListNode*> path;
				FindNodesInPathway(stmtNodeA, stmtNodeB, path);
				dList<dCIL::dListNode*>& statementList = definedStatements->GetInfo();
				for (dList<dCIL::dListNode*>::dListNode* ptr = statementList.GetFirst(); ptr; ptr = ptr->GetNext()) {
					dCIL::dListNode* const stmt = ptr->GetInfo();

//DTRACE_INTRUCTION (&stmt);		
					if (path.Find(stmt)) {
						return false;
					}
				}
				
				if ((constStmt.m_instruction == dThreeAdressStmt::m_load) || ((constStmt.m_operator != dThreeAdressStmt::m_nothing) && ((constStmt.m_arg2.GetType().m_intrinsicType == dThreeAdressStmt::m_int) || (constStmt.m_arg2.GetType().m_intrinsicType == dThreeAdressStmt::m_float)))) {
					dTree<dList<dCIL::dListNode*>, dString>::dTreeNode* const definedStatements = m_variableDefinitions.Find(constStmt.m_arg2.m_label);
					if (definedStatements) {
						dList<dCIL::dListNode*>& statementList = definedStatements->GetInfo();
						for (dList<dCIL::dListNode*>::dListNode* ptr = statementList.GetFirst(); ptr; ptr = ptr->GetNext()) {
							dCIL::dListNode* const stmt = ptr->GetInfo();
							if (path.Find(stmt)) {
								return false;
							}
						}
					}
				}
			}
		}
	}
*/
	return canApplyPropagation;
}


bool dDataFlowGraph::ApplyCopyPropagation()
{
	bool ret = false;
	for (dCIL::dListNode* node = m_basicBlocks.m_begin; node != m_basicBlocks.m_end; node = node->GetNext()) {
		dCILInstrMove* const moveInst = node->GetInfo()->GetAsMove();
		if (moveInst) {
			moveInst->Trace();
			for (dCIL::dListNode* node1 = node->GetNext(); node1 != m_basicBlocks.m_end; node1 = node1->GetNext()) {
				dCILInstr* const inst = node1->GetInfo();
				if (DoStatementAreachesStatementB(node1, node)) {
					inst->Trace();
					ret |= inst->ApplyCopyPropagation(moveInst, *this);
					inst->Trace();
				}
			}
		}

		/*
		switch (stmt.m_instruction)
		{
			case dThreeAdressStmt::m_assigment:
			{
				//if ((stmt.m_operator == dThreeAdressStmt::m_nothing) && (stmt.m_arg1.m_type == dThreeAdressStmt::m_intVar)) {
				if ((stmt.m_operator == dThreeAdressStmt::m_nothing) && (stmt.m_arg1.GetType().m_intrinsicType != dThreeAdressStmt::m_constInt) && (stmt.m_arg1.GetType().m_intrinsicType != dThreeAdressStmt::m_constFloat)){
					for (dCIL::dListNode* stmtNode1 = stmtNode->GetNext(); stmtNode1; stmtNode1 = stmtNode1->GetNext()) {
						dThreeAdressStmt& stmt1 = stmtNode1->GetInfo();
//DTRACE_INTRUCTION (&stmt1);		
						switch (stmt1.m_instruction)
						{
							case dThreeAdressStmt::m_assigment:
							{
								if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)){
									ret = true;
									stmt1.m_arg1 = stmt.m_arg1;
									UpdateReachingDefinitions();
								}
								if ((stmt1.m_operator != dThreeAdressStmt::m_nothing) && (stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;
									stmt1.m_arg2 = stmt.m_arg1;
									UpdateReachingDefinitions();
								}
								break;
							}

							case dThreeAdressStmt::m_load:
							{
								if ((stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;		
									stmt1.m_arg2 = stmt.m_arg1;
									UpdateReachingDefinitions();
								} else if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;		
									stmt1.m_arg1 = stmt.m_arg1;
									UpdateReachingDefinitions();
								}
								break;
							}

							case dThreeAdressStmt::m_storeBase:
							{
								if ((stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;
									stmt1.m_arg0 = stmt.m_arg1;
									UpdateReachingDefinitions();
								}
								break;
							}

							case dThreeAdressStmt::m_store:
							{
								if ((stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;
									stmt1.m_arg0 = stmt.m_arg1;
									UpdateReachingDefinitions();
								} else if ((stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;		
									stmt1.m_arg2 = stmt.m_arg1;
									UpdateReachingDefinitions();
								} else if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;		
									stmt1.m_arg1 = stmt.m_arg1;
									UpdateReachingDefinitions();
								}
								break;
							}

							case dThreeAdressStmt::m_if:
							{
								if ((stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)){
									ret = true;
									stmt1.m_arg0 = stmt.m_arg1;
									UpdateReachingDefinitions();
								}
								if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
									ret = true;
									stmt1.m_arg1 = stmt.m_arg1;
									UpdateReachingDefinitions();
								}
								break;
							}
						}
					}
				}
				break;
			}
#if 0
            case dThreeAdressStmt::m_storeBase:
            {
                for (dCIL::dListNode* stmtNode1 = stmtNode->GetNext(); stmtNode1; stmtNode1 = stmtNode1->GetNext()) {
                    dThreeAdressStmt& stmt1 = stmtNode1->GetInfo();
//DTRACE_INTRUCTION (&stmt1);		
                    switch (stmt1.m_instruction)
                    {
                        case dThreeAdressStmt::m_assigment:
                        {
                            if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)){
                                ret = true;
                                stmt1.m_arg1 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            }
                            if ((stmt1.m_operator != dThreeAdressStmt::m_nothing) && (stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;
                                stmt1.m_arg2 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            }
                            break;
                        }

                        case dThreeAdressStmt::m_load:
                        {
                            if ((stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;		
                                stmt1.m_arg2 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            } else if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;		
                                stmt1.m_arg1 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            }
                            break;
                        }

                        case dThreeAdressStmt::m_loadBase:
                        {
                            if ((stmt1.m_arg2.m_label == stmt.m_arg2.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;
                                stmt1.m_arg1 = stmt.m_arg0;
								stmt1.m_arg2.m_label = dString("");
								stmt1.m_instruction = dThreeAdressStmt::m_assigment;
								stmt1.m_operator = dThreeAdressStmt::m_nothing;
                                UpdateReachingDefinitions();
                            }
                            break;
                        }

                        case dThreeAdressStmt::m_store:
                        {
                            if ((stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;
                                stmt1.m_arg0 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            } else if ((stmt1.m_arg2.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;		
                                stmt1.m_arg2 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            } else if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;		
                                stmt1.m_arg1 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            }
                            break;
                        }

                        case dThreeAdressStmt::m_if:
                        {
                            if ((stmt1.m_arg0.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)){
                                ret = true;
                                stmt1.m_arg0 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            }
                            if ((stmt1.m_arg1.m_label == stmt.m_arg0.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)) {
                                ret = true;
                                stmt1.m_arg1 = stmt.m_arg0;
                                UpdateReachingDefinitions();
                            }
                            break;
                        }
                    }
                }

                break;
            }

			case dThreeAdressStmt::m_load:
			{
				#ifdef D_OPTIMIZE_REDUNDANCE_COPY_ON_ARRAYS
					for (dCIL::dListNode* stmtNode1 = stmtNode->GetNext(); stmtNode1; stmtNode1 = stmtNode1->GetNext()) {
						dThreeAdressStmt& stmt1 = stmtNode1->GetInfo();
						switch (stmt1.m_instruction)
						{
							case dThreeAdressStmt::m_load:
							{
								if ((stmt1.m_arg1.m_label == stmt.m_arg1.m_label) && (stmt1.m_arg2.m_label == stmt.m_arg2.m_label) && DoStatementAreachesStatementB(stmtNode1, stmtNode)){
									ret = true;
									stmt1.m_instruction = dThreeAdressStmt::m_assigment;
									stmt1.m_operator = dThreeAdressStmt::m_nothing;
									stmt1.m_arg1 = stmt.m_arg0;
									stmt1.m_arg2.m_label = "";
									stmt1.m_extraInformation = 0;
									UpdateReachingDefinitions();
								}
								break;
							}
						}
					}
				#endif
				break;
			}
#endif
		}
*/
	}

	return ret;
}

bool dDataFlowGraph::ApplyInstructionSematicOrdering()
{
	bool ret = false;
	for (dCIL::dListNode* stmtNode = m_function->GetNext(); stmtNode && !stmtNode->GetInfo()->GetAsFunction(); stmtNode = stmtNode->GetNext()) {
		ret |= stmtNode->GetInfo()->ApplySemanticReordering ();
	}
	return ret;
}


bool dDataFlowGraph::ApplyRemoveDeadCode()
{
	CalculateLiveInputLiveOutput ();
	bool ret = false;
	dCIL::dListNode* nextNode;
	for (dCIL::dListNode* node = m_basicBlocks.m_begin; node != m_basicBlocks.m_end; node = nextNode) {
		nextNode = node->GetNext();
		dCILInstr* const instr = node->GetInfo();
		if (instr->CanBeEliminated()) {
			if (instr->GetAsMove()) {
				dCILInstrMove* const move = instr->GetAsMove();
				dDataFlowPoint& info = m_dataFlowGraph.Find(node)->GetInfo();
				dDataFlowPoint::dVariableSet<dString>& liveOut = info.m_liveOutputSet;
				const dCILInstr::dArg arg0 = move->GetArg0();
				const dCILInstr::dArg arg1 = move->GetArg1();
				bool xxx = liveOut.Find(arg0.m_label);
				if (!liveOut.Find(arg0.m_label) || (arg0.m_label == arg1.m_label)) {
					ret = true;
					//m_dataFlowGraph.Remove(node);
					//delete move;
					move->Nullify();
					UpdateLiveInputLiveOutput();
				}
			}
	/*
		dThreeAdressStmt& stmt = stmtNode->GetInfo();
		switch (stmt.m_instruction) 
		{
			case dThreeAdressStmt::m_assigment:
			{
				dDataFlowPoint& info = m_dataFlowGraph.Find(stmtNode)->GetInfo();
				dDataFlowPoint::dVariableSet<dString>& liveOut = info.m_liveOutputSet;
				if (!liveOut.Find (stmt.m_arg0.m_label)) {
					ret = true;
					stmt.m_instruction = dThreeAdressStmt::m_nop;
					UpdateLiveInputLiveOutput();
				} else if ((stmt.m_operator == dThreeAdressStmt::m_nothing) && (stmt.m_arg0.m_label == stmt.m_arg1.m_label)){
					ret = true;
					stmt.m_instruction = dThreeAdressStmt::m_nop;
					UpdateLiveInputLiveOutput();
				}
				break;
			}
#if 0
			case dThreeAdressStmt::m_load:
			case dThreeAdressStmt::m_loadBase:
			{
				dDataFlowPoint& info = m_dataFlowGraph.Find(stmtNode)->GetInfo();
				dDataFlowPoint::dVariableSet<dString>& liveOut = info.m_liveOutputSet;
				if (!liveOut.Find (stmt.m_arg0.m_label)) {
					ret = true;
					stmt.m_instruction = dThreeAdressStmt::m_nop;
					UpdateLiveInputLiveOutput();
				}
				break;
			}

			case dThreeAdressStmt::m_store:
			{
				dAssert (0);
				break;
			}

			case dThreeAdressStmt::m_storeBase:
			{
				dDataFlowPoint& info = m_dataFlowGraph.Find(stmtNode)->GetInfo();
				dDataFlowPoint::dVariableSet<dString>& liveOut = info.m_liveOutputSet;
				if (!liveOut.Find (stmt.m_arg0.m_label)) {
					ret = true;
					stmt.m_instruction = dThreeAdressStmt::m_nop;
					UpdateLiveInputLiveOutput();
				}
				break;
			}
#endif
*/
		}
	}
	return ret;
}

bool dDataFlowGraph::RemoveDeadInstructions()
{
dAssert (0);
return 0;
/*
	bool ret = false;
	int count = 0;
	for (dCIL::dListNode* stmtNode = m_basicBlocks.m_begin->GetNext()->GetNext(); (stmtNode != m_basicBlocks.m_end) && (stmtNode->GetInfo().m_instruction == dThreeAdressStmt::m_argument) && (count < D_CALLER_SAVE_REGISTER_COUNT); stmtNode = stmtNode->GetNext()) {
		dThreeAdressStmt& stmt = stmtNode->GetInfo();
		stmt.m_instruction = dThreeAdressStmt::m_nop;
		count ++;
	}

	for (dCIL::dListNode* stmtNode = m_basicBlocks.m_begin->GetNext()->GetNext(); stmtNode != m_basicBlocks.m_end; stmtNode = stmtNode->GetNext()) {
		dThreeAdressStmt& stmt = stmtNode->GetInfo();
		if (stmt.m_instruction == dThreeAdressStmt::m_call) {
			int count = 0;
			for (dCIL::dListNode* node = stmtNode->GetPrev(); (node->GetInfo().m_instruction == dThreeAdressStmt::m_param) && (count < D_CALLER_SAVE_REGISTER_COUNT); node = node->GetPrev()) {
				dThreeAdressStmt& stmt1 = node->GetInfo();
				stmt1.m_instruction = dThreeAdressStmt::m_nop;
				count ++;
			}
		}
	}

	return ret;
*/
}

bool dDataFlowGraph::RemoveNop()
{
dAssert (0);
return 0;
/*
	bool ret = false;
	dCIL::dListNode* nextStmtNode;
	for (dCIL::dListNode* stmtNode = m_basicBlocks.m_begin; stmtNode != m_basicBlocks.m_end; stmtNode = nextStmtNode) {
		nextStmtNode = stmtNode->GetNext();
		dThreeAdressStmt& stmt = stmtNode->GetInfo();	
		if (stmt.m_instruction == dThreeAdressStmt::m_nop) {
			m_cil->Remove(stmtNode);
			ret = true;
		}
	}
	return ret;
*/
}


bool dDataFlowGraph::RemoveRedundantJumps ()
{
	bool ret = false;
	dTree<int, dCIL::dListNode*> jumpMap;

	// create jump and label map;
	for (dCIL::dListNode* stmtNode = m_basicBlocks.m_begin; stmtNode != m_basicBlocks.m_end; stmtNode = stmtNode->GetNext()) {
		dCILInstr* const instr = stmtNode->GetInfo();
		if (instr->GetAsIF() || instr->GetAsLabel() || instr->GetAsGoto()) {
			jumpMap.Insert(0, stmtNode);
		}
	}


	dTree<int, dCIL::dListNode*>::Iterator iter (jumpMap);

	// remove if else to immediate label
	for (iter.Begin(); iter; iter ++) {
		dCIL::dListNode* const node = iter.GetKey();
		//dThreeAdressStmt& stmt = node->GetInfo();
		dCILInstr* const instr = node->GetInfo();
		//if (stmt.m_instruction == dThreeAdressStmt::m_if) {
		if (instr->GetAsIF()) {
			dCILInstrIFNot* const ifInstr = instr->GetAsIF();
			dCIL::dListNode* const elseTarget = ifInstr->GetFalseTarget();
			if (elseTarget) {
				dAssert(jumpMap.Find(elseTarget));
				dCIL::dListNode* const falseTargetJump = jumpMap.Find(elseTarget)->GetKey();
				//dThreeAdressStmt& stmt1 = falseTargetJump->GetInfo();

				dCILInstr* const instr1 = falseTargetJump->GetInfo();
				dCIL::dListNode* nextGotoNode = node->GetNext();
				while (nextGotoNode->GetInfo()->GetAsNop()) {
					nextGotoNode = nextGotoNode->GetNext();
				}

				dCILInstrLabel* const const label = nextGotoNode->GetInfo()->GetAsLabel();
				//dThreeAdressStmt& nextStmt = nextGotoNode->GetInfo();
				dAssert(label);
				if (nextGotoNode == falseTargetJump) {
					//stmt.m_falseTargetJump = NULL;
					//stmt.m_arg2.m_label = "";
					ifInstr->SetTargets(ifInstr->GetTrueTarget()->GetInfo()->GetAsLabel(), NULL);
					label->Nullify();
				}
			}
		}
	}


#if 0
	// remove redundant adjacent labels
	
	for (iter.Begin(); iter; iter ++) {
		dCIL::dListNode* const node = iter.GetKey();
		const dThreeAdressStmt& stmt = node->GetInfo();

		if (stmt.m_instruction == dThreeAdressStmt::m_label) {
			dCIL::dListNode* const labelNode = node->GetNext();
			if (labelNode && (labelNode->GetInfo().m_instruction == dThreeAdressStmt::m_label)) {
				dTree<int, dCIL::dListNode*>::Iterator iter1 (jumpMap);
				for (iter1.Begin(); iter1; iter1 ++) {
					dCIL::dListNode* const node1 = iter1.GetKey();
					dThreeAdressStmt& stmt1 = node1->GetInfo();
					if (stmt1.m_instruction == dThreeAdressStmt::m_goto) {
						dAssert (0);
//						if (stmt1.m_jmpTarget == labelNode)	{
//							stmt1.m_jmpTarget = node;
//							stmt1.m_arg0.m_label = stmt.m_arg0.m_label;
//						}
					} else if (stmt1.m_instruction == dThreeAdressStmt::m_if) { 
						dAssert (0);
//						if (stmt1.m_jmpTarget == labelNode)	{
//							stmt1.m_jmpTarget = node;	
//							stmt1.m_arg2.m_label = stmt.m_arg0.m_label;
//						}
					}
				}
				ret = true;
				dAssert (0);
				//m_cil->Remove(labelNode);
				//jumpMap.Remove(labelNode);
			}
		}
	}


	// redirect double indirect goto
	for (iter.Begin(); iter; iter ++) {
		dCIL::dListNode* const node = iter.GetKey();
		dThreeAdressStmt& stmt = node->GetInfo();
DTRACE_INTRUCTION (&stmt);
		if (stmt.m_instruction == dThreeAdressStmt::m_goto) {
			dAssert (jumpMap.Find (stmt.m_trueTargetJump));
			dCIL::dListNode* const targetNode = jumpMap.Find (stmt.m_trueTargetJump)->GetKey();
			dThreeAdressStmt& stmt1 = targetNode->GetInfo();
			dCIL::dListNode* nextGotoNode = targetNode->GetNext();
			while (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_nop) {
				nextGotoNode = nextGotoNode->GetNext();
			}
			if ((stmt1.m_instruction == dThreeAdressStmt::m_label) && (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_goto)) {
				const dThreeAdressStmt& stmt2 = nextGotoNode->GetInfo();
				stmt.m_arg0.m_label = stmt2.m_arg0.m_label;
				stmt.m_trueTargetJump = stmt2.m_trueTargetJump;
				ret = true;
			}

		} else if (stmt.m_instruction == dThreeAdressStmt::m_if) {
			if (stmt.m_falseTargetJump) {
				dAssert (jumpMap.Find (stmt.m_falseTargetJump));
				dCIL::dListNode* const falseTargetJump = jumpMap.Find (stmt.m_falseTargetJump)->GetKey();
				dThreeAdressStmt& stmt1 = falseTargetJump->GetInfo();

				dCIL::dListNode* nextGotoNode = falseTargetJump->GetNext();
				while (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_nop) {
					nextGotoNode = nextGotoNode->GetNext();
				}

				dThreeAdressStmt& nextStmt = nextGotoNode->GetInfo();
				dAssert (nextStmt.m_instruction != dThreeAdressStmt::m_label);
				stmt.m_falseTargetJump = NULL;
				stmt.m_arg2.m_label = "";
			}
#if 0
			dAssert (jumpMap.Find (stmt.m_trueTargetJump));
			dCIL::dListNode* const trueTargetNode = jumpMap.Find (stmt.m_trueTargetJump)->GetKey();
			dThreeAdressStmt& stmt1 = trueTargetNode->GetInfo();

			dCIL::dListNode* nextGotoNode = trueTargetNode->GetNext();
			while (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_nop) {
				nextGotoNode = nextGotoNode->GetNext();
			}
			if ((stmt1.m_instruction == dThreeAdressStmt::m_label) && (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_goto)) {
				dAssert (0);

				const dThreeAdressStmt& stmt2 = nextGotoNode->GetInfo();
				stmt.m_arg2.m_label = stmt2.m_arg0.m_label;
				stmt.m_jmpTarget = stmt2.m_jmpTarget;
				ret = true;
			}
#endif
		}
	}
	

	// remove goto to immediate labels
	for (iter.Begin(); iter; iter ++) {
		dCIL::dListNode* const node = iter.GetKey();
		dThreeAdressStmt& stmt = node->GetInfo();
		dCIL::dListNode* const nextNode = node->GetNext();
		dAssert (0);

		//if (((stmt.m_instruction == dThreeAdressStmt::m_if) || (stmt.m_instruction == dThreeAdressStmt::m_goto)) && (stmt.m_jmpTarget == nextNode)) {
			dAssert (0);
			ret = true;
			//Remove(node);
			//jumpMap.Remove(node);
		//}
	}

#endif

dAssert(0);
return 0;
/*
	// redirect double indirect if
	for (iter.Begin(); iter; iter ++) {
		dCIL::dListNode* const node = iter.GetKey();
		dThreeAdressStmt& stmt = node->GetInfo();
//DTRACE_INTRUCTION (&stmt);
		if (stmt.m_instruction == dThreeAdressStmt::m_if) {
			dAssert (jumpMap.Find (stmt.m_trueTargetJump));
			dCIL::dListNode* const trueTargetNode = jumpMap.Find (stmt.m_trueTargetJump)->GetKey();
			//dThreeAdressStmt& stmt1 = trueTargetNode->GetInfo();

			dCIL::dListNode* nextGotoNode = trueTargetNode;
			while ((nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_nop) || (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_label)) {
				nextGotoNode = nextGotoNode->GetNext();
			}

			dThreeAdressStmt& stmt1 = nextGotoNode->GetInfo();
			if (stmt1.m_instruction == dThreeAdressStmt::m_goto) {
				//const dThreeAdressStmt& stmt2 = nextGotoNode->GetInfo();
				stmt.m_arg1.m_label = stmt1.m_arg0.m_label;
				stmt.m_trueTargetJump = stmt1.m_trueTargetJump;
				ret = true;
			}
		}
	}


	// delete goto to immidiate label
	for (iter.Begin(); iter; iter ++) {
		dCIL::dListNode* const node = iter.GetKey();
		dThreeAdressStmt& stmt = node->GetInfo();
		if (stmt.m_instruction == dThreeAdressStmt::m_goto) {
			dCIL::dListNode* const trueTargetNode = jumpMap.Find (stmt.m_trueTargetJump)->GetKey();
			dCIL::dListNode* nextGotoNode = trueTargetNode;
			while (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_nop) {
				nextGotoNode = nextGotoNode->GetNext();
			}
			dThreeAdressStmt& stmt1 = nextGotoNode->GetInfo();
			dAssert (stmt1.m_instruction == dThreeAdressStmt::m_label);
			if (stmt1.m_trueTargetJump == nextGotoNode) {
				ret = true;
				stmt.m_instruction = dThreeAdressStmt::m_nop;
				jumpMap.Remove(node);
			}
		}
	}

	// delete unreacheble goto
	for (iter.Begin(); iter; iter ++) {
		dCIL::dListNode* const node = iter.GetKey();
		dThreeAdressStmt& stmt = node->GetInfo();
		if (stmt.m_instruction == dThreeAdressStmt::m_goto) {
			//dCIL::dListNode* const trueTargetNode = jumpMap.Find (stmt.m_trueTargetJump)->GetKey();
			dCIL::dListNode* prevNode = node->GetPrev();
			while (prevNode->GetInfo().m_instruction == dThreeAdressStmt::m_nop) {
				prevNode = prevNode->GetPrev();
			}
			dThreeAdressStmt& stmt1 = prevNode->GetInfo();
			if ((stmt1.m_instruction == dThreeAdressStmt::m_ret) || (stmt1.m_instruction == dThreeAdressStmt::m_goto)){
				ret = true;
				stmt.m_instruction = dThreeAdressStmt::m_nop;
				jumpMap.Remove(node);
			}
		}
	}


	// redirect if to immideate label
	for (iter.Begin(); iter; ) {
		dCIL::dListNode* const node = iter.GetKey();
		dThreeAdressStmt& stmt = node->GetInfo();
		iter ++;
//DTRACE_INTRUCTION (&stmt);
		if (stmt.m_instruction == dThreeAdressStmt::m_if) {
			dAssert (jumpMap.Find (stmt.m_trueTargetJump));
			dCIL::dListNode* const trueTargetNode = jumpMap.Find (stmt.m_trueTargetJump)->GetKey();
			dCIL::dListNode* nextGotoNode = trueTargetNode;
			while ((nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_nop) || (nextGotoNode->GetInfo().m_instruction == dThreeAdressStmt::m_label)) {
				nextGotoNode = nextGotoNode->GetNext();
			}

			bool islive = false;
			for (dCIL::dListNode* node1 = node->GetNext(); node1 != nextGotoNode; node1 = node1->GetNext()) {
				dThreeAdressStmt& stmt1 = node1->GetInfo();
				if (!((stmt1.m_instruction == dThreeAdressStmt::m_nop) || (stmt1.m_instruction == dThreeAdressStmt::m_label))) {
					islive = true;
					break;
				}
			}
			if (!islive) {
				stmt.m_instruction = dThreeAdressStmt::m_nop;
				jumpMap.Remove(node);
				ret = true;
			}
		}
	}

	// delete unreferenced labels
	for (iter.Begin(); iter; ) {
		dCIL::dListNode* const node = iter.GetKey();
		dThreeAdressStmt& stmt = node->GetInfo();
		iter ++;
//DTRACE_INTRUCTION (&stmt);		
		if (stmt.m_instruction == dThreeAdressStmt::m_label) {		
			dTree<int, dCIL::dListNode*>::Iterator iter1 (jumpMap);
			bool isReferenced = false;
			for (iter1.Begin(); iter1; iter1 ++) {
				dCIL::dListNode* const node1 = iter1.GetKey();
				dThreeAdressStmt& stmt1 = node1->GetInfo();
				if (stmt1.m_instruction == dThreeAdressStmt::m_goto){
					if (stmt1.m_trueTargetJump == node) {
						isReferenced = true;
						break;
					}
				} else if (stmt1.m_instruction == dThreeAdressStmt::m_if) {
					if ((stmt1.m_trueTargetJump == node) || (stmt1.m_falseTargetJump == node)) {
						isReferenced = true;
						break;
					}
				}
			}
			if (!isReferenced) {
				ret = true;
				stmt.m_instruction = dThreeAdressStmt::m_nop;
				jumpMap.Remove(node);
			}
		}
	}


//m_cil->Trace();	
	return ret;
*/
}


void dDataFlowGraph::RegistersAllocation (int registerCount)
{
	dRegisterInterferenceGraph interferenceGraph(this, registerCount);
}
