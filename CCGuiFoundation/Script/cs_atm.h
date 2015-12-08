#ifndef CS_ATM
#define CS_ATM

#include "stdafx.h"
#include "cs_inst.h"
#include "cs_pool.h"

using namespace std;
using namespace cc::base;

namespace cc
{
	namespace script
	{
		template<typename _EdgeData, typename _StatusData>
		class Cs_AtmEdge;
		template<typename _EdgeData, typename _StatusData>
		class Cs_AtmStatus;
		template<typename _EdgeData, typename _StatusData>
		class Cs_AtmIBFSAlgorithm;

		template<typename _EdgeData, typename _StatusData>
		class Cs_AtmEdge
		{
			using _Cs_AtmStatus = Cs_AtmStatus<_EdgeData, _StatusData>;
		public:
			RawPtr<_Cs_AtmStatus>		_Beg;
			RawPtr<_Cs_AtmStatus>		_End;
			_EdgeData					_Data;
		};

		template<typename _EdgeData, typename _StatusData>
		class Cs_AtmStatus
		{
			using _Cs_AtmEdge = Cs_AtmEdge<_EdgeData, _StatusData>;
		public:
			typedef vector<_Cs_AtmEdge*>_EdgeList;

			_EdgeList					_In_edges;
			_EdgeList					_Out_edges;
			_StatusData					_Data;

			void Visit(Cs_AtmIBFSAlgorithm<_EdgeData, _StatusData>* _Al)
			{
				/*��ʼ��·������������*/
				auto & _Stack = _Al->_Path;
				_Stack.clear();
				_Stack.push_back(this);
				/*���п����������*/
				for (auto & _Cur : _Stack)
				{
					bool _Visit_connected = true;
					bool _Run_end = true;
					/*����״̬*/
					_Al->VisitBegin(_Cur, _Visit_connected, _Run_end);
					if (_Visit_connected)
					{
						/*ȡ����ǰ״̬��ÿһ����*/
						for (auto & CurrentEdge : _Cur->_Out_edges)
						{
							/*����ߵ�Ŀ����δ���ʣ����ұ�ͨ������*/
							if (find(_Stack.begin(), _Stack.end(), CurrentEdge->_End) == _Stack.end() && _Al->EdgeTest(CurrentEdge))
							{
								/*��·���ϼ���ߵ�Ŀ�꣬�Դ���������*/
								_Stack.push_back(CurrentEdge->_End);
							}
						}
					}
					if (_Run_end)
					{
						/*��������״̬*/
						_Al->VisitEnd(_Cur);
					}
				}
			}
		};

		/************************************************************************/
		/* ���ڹ�������������Զ����㷨�ӿ�                                     */
		/************************************************************************/
		template<typename _EdgeData, typename _StatusData>
		class Cs_AtmIBFSAlgorithm
		{
			using _Cs_AtmStatus = Cs_AtmStatus<_EdgeData, _StatusData>;
			using _Cs_AtmEdge = Cs_AtmEdge<_EdgeData, _StatusData>;
		public:
			virtual bool EdgeTest(_Cs_AtmEdge*)
			{
				return true;
			}

			virtual void VisitBegin(_Cs_AtmStatus*, bool&, bool&)
			{
			}
			virtual void VisitEnd(_Cs_AtmStatus*)
			{
			}

			vector<_Cs_AtmStatus*> _Path;
		};

		template<typename _EdgeData, typename _StatusData, size_t _Size = POOL_SIZE>
		class Cs_Automata
		{
		protected:
			using _Cs_AtmStatus = Cs_AtmStatus<_EdgeData, _StatusData>;
			using _Cs_AtmEdge = Cs_AtmEdge<_EdgeData, _StatusData>;

			class Initer
			{
				friend class Cs_Automata;
			protected:
				Cs_Automata*					_Au;
			public:
				void init(_Cs_AtmEdge* _Data)
				{
					_Au->init(_Data);
				}
				void init(_Cs_AtmStatus* _Data)
				{
					_Au->init(_Data);
				}
			};

		public:
			Cs_Pool<_Cs_AtmEdge, Initer, _Size>			_Edge_pool;
			Cs_Pool<_Cs_AtmStatus, Initer, _Size>		_Status_pool;

			Cs_Automata()
			{
				_Edge_pool._Get_initializer()->_Au = this;
				_Status_pool._Get_initializer()->_Au = this;
			}

			virtual void init(_Cs_AtmEdge* Data)
			{
			}

			virtual void init(_Cs_AtmStatus* _Data)
			{
			}

			_Cs_AtmEdge* connect(_Cs_AtmStatus* Start, _Cs_AtmStatus* End)
			{
				_Cs_AtmEdge* _Edge = _Edge_pool.alloc();
				_Edge->_Beg = _Beg;
				_Edge->_End = _End;
				Start->_Out_edges.Add(_Edge);
				End->_In_edges.Add(_Edge);
				return _Edge;
			}

			void disconnect(_Cs_AtmStatus* Start, _Cs_AtmEdge* Edge)
			{
				auto _Idx = find(Start->_Out_edges.begin(), Start->_Out_edges.end(), Edge);
				if (_Idx != Start->_Out_edges.end())
				{
					Start->_Out_edges.erase(_Idx);
					Edge->_End->_In_edges.erase(find(Edge->_End->_In_edges.begin(), Edge->_End->_In_edges.end(), Edge));
					_Edge_pool.free(Edge);
				}
			}

			void disconnect(_Cs_AtmStatus* Start)
			{
				/*ɾ�����״̬���ӵ����б�*/
				for (auto & InEdge : Start->_In_edges)
				{
					disconnect(InEdge->_Beg, InEdge);
				}
				for (auto & OutEdge : Start->_Out_edges)
				{
					disconnect(Start, OutEdge);
				}
				/*ɾ����״̬*/
				_Status_pool.free(Start);
			}
		};
	}
}

#endif