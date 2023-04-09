#pragma once

#include "core.h"
#include "base_io.h"
#include "buffer_io.h"
#include "include/nlohmann/json.hpp"

/*
* @brief RPC��ɫ����
*/
enum rpc_role_type {  
	RPC_CLIENT,
	RPC_SERVER
};

/*
* @brief RPC�����࣬�������л��ͷ����л�
*/
class RPCData {
public:
	std::string protocol_name;  /*Э������*/
	int param_number;  /*��������*/
public:
	std::string function_name;  /*Զ�̵��ú�������*/
	int param1;
	int param2;
	int retval;

public:
	/*
	* @brief Ĭ�Ϲ��캯��
	*/
	RPCData() = default;

	/*
	* @brief �вι��캯��
	*/
	RPCData(std::string _name, int _p1, int _p2) :
		protocol_name("RPC1.0"), param_number(2), retval(0),
		function_name(_name), param1(_p1), param2(_p2) {}

	/*
	* @brief ����תjson
	*/
	nlohmann::json to_json() {
		nlohmann::json j = nlohmann::json{
			{"protocol_name", protocol_name},
			{"param_number", param_number},
			{"function_name", function_name},
			{"param1", param1},
			{"param2", param2},
			{"retval", retval}
		};
		return j;
	}

	/*
	* @brief jsonת����
	*/
	void from_json(const nlohmann::json& j) {
		j.at("protocol_name").get_to(protocol_name);
		j.at("param_number").get_to(param_number);
		j.at("function_name").get_to(function_name);
		j.at("param1").get_to(param1);
		j.at("param2").get_to(param2);
		j.at("retval").get_to(retval);
	}
};


/*
* @brief RPC�࣬���ڴ���RPC�߼�
*/
class RPCStub {
private:
	using function_type = int(*)(int, int);  /*��������*/
private:
	rpc_role_type role;  /*RPC��ɫ*/	
	std::unordered_map<std::string, function_type> stub_map;  /*���ش��*/

public:
	/*
	* @brief ���캯��
	*/
	RPCStub(rpc_role_type _role) :role(_role) {}

public:
	/*
	* @brief ���Ӻ�����̬�󶨵����ش������������
	*/
	void add_stub(std::string _name, function_type _func) {
		stub_map[_name] = _func;
	}

	/*
	* @brief ����Զ�̵������󣨷�������
	*/
	int rpc_call_server(int fd) {
		char buffer[core::MAX_LINE];  /*��������*/
		memset(buffer, '\0', core::MAX_LINE);

		// ��fd�ж���RPC����
		if (recv(fd, buffer, core::MAX_LINE, 0) <= 0) {
#ifdef DEBUG
			printf("fd %d no message received\n", fd);
#endif // DEBUG	
			return -1;
		}

#ifdef DEBUG
		std::string buffer_s(buffer);
		printf("receive: %s\n", buffer_s.c_str());
#endif // DEBUG		

		// ��bufferתΪjson����
		if (!nlohmann::json::accept(buffer)) {
			printf("receive json cannot be parsed.\n");
			return 0;
		}
		nlohmann::json json_data = nlohmann::json::parse(buffer);		

		// ��json����תΪData����
		RPCData data;
		data.from_json(json_data);

		// ���ú���ӳ�䴦��
		function_type callback = stub_map[data.function_name];
		data.retval = callback(data.param1, data.param2);

		// ��Data����תΪjson����
		json_data = data.to_json();
		std::string json_string = json_data.dump();
#ifdef DEBUG
		printf("send: %s\n", json_string.c_str());
#endif // DEBUG		

		// ���ͽ�����ͻ���
		memset(buffer, '\0', core::MAX_LINE);
		sprintf(buffer, "%s", json_string.c_str());
		int send_n = send(fd, buffer, strlen(buffer), 0);
		printf("send num: %d\n", send_n);

		return 0;
	}

	/*
	* @brief ����Զ�̵��ã��ͻ��ˣ�
	*/
	int rpc_call_client(int fd, std::string func_name, int p1, int p2) {
		char buffer[core::MAX_LINE];  /*��������*/
		memset(buffer, '\0', core::MAX_LINE);

		// ����Data����
		RPCData data(func_name, p1, p2);

		// ��Data����תΪjson����
		nlohmann::json json_data = data.to_json();
		std::string json_string = json_data.dump();
		// ���ͽ�����ͻ���
		sprintf(buffer, "%s", json_string.c_str());
		send(fd, buffer, strlen(buffer), 0);

#ifdef DEBUG
		printf("send: %s\n", json_string.c_str());
#endif // DEBUG	

		// ��fd�ж���RPC��Ӧ
		memset(buffer, '\0', core::MAX_LINE);
		if (recv(fd, buffer, core::MAX_LINE, 0) == -1)
		{
			printf("receive error\n");
		}

#ifdef DEBUG
		std::string buffer_s(buffer);
		printf("receive: %s\n", buffer_s.c_str());
#endif // DEBUG	

		// ��bufferתΪjson����
		if (!nlohmann::json::accept(buffer)) {
			printf("receive json cannot be parsed.\n");
			return 0;
		}
		json_data = nlohmann::json::parse(buffer);
		// ��json����תΪData����
		data.from_json(json_data);

		return data.retval;
	}
};
