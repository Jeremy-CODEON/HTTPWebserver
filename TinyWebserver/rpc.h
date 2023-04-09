#pragma once

#include "core.h"
#include "base_io.h"
#include "buffer_io.h"
#include "include/nlohmann/json.hpp"

/*
* @brief RPC角色类型
*/
enum rpc_role_type {  
	RPC_CLIENT,
	RPC_SERVER
};

/*
* @brief RPC数据类，用于序列化和反序列化
*/
class RPCData {
public:
	std::string protocol_name;  /*协议名称*/
	int param_number;  /*参数个数*/
public:
	std::string function_name;  /*远程调用函数名称*/
	int param1;
	int param2;
	int retval;

public:
	/*
	* @brief 默认构造函数
	*/
	RPCData() = default;

	/*
	* @brief 有参构造函数
	*/
	RPCData(std::string _name, int _p1, int _p2) :
		protocol_name("RPC1.0"), param_number(2), retval(0),
		function_name(_name), param1(_p1), param2(_p2) {}

	/*
	* @brief 对象转json
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
	* @brief json转对象
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
* @brief RPC类，用于处理RPC逻辑
*/
class RPCStub {
private:
	using function_type = int(*)(int, int);  /*函数类型*/
private:
	rpc_role_type role;  /*RPC角色*/	
	std::unordered_map<std::string, function_type> stub_map;  /*本地存根*/

public:
	/*
	* @brief 构造函数
	*/
	RPCStub(rpc_role_type _role) :role(_role) {}

public:
	/*
	* @brief 增加函数动态绑定到本地存根（服务器）
	*/
	void add_stub(std::string _name, function_type _func) {
		stub_map[_name] = _func;
	}

	/*
	* @brief 处理远程调用请求（服务器）
	*/
	int rpc_call_server(int fd) {
		char buffer[core::MAX_LINE];  /*读缓冲区*/
		memset(buffer, '\0', core::MAX_LINE);

		// 从fd中读入RPC请求
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

		// 从buffer转为json对象
		if (!nlohmann::json::accept(buffer)) {
			printf("receive json cannot be parsed.\n");
			return 0;
		}
		nlohmann::json json_data = nlohmann::json::parse(buffer);		

		// 从json对象转为Data对象
		RPCData data;
		data.from_json(json_data);

		// 调用函数映射处理
		function_type callback = stub_map[data.function_name];
		data.retval = callback(data.param1, data.param2);

		// 从Data对象转为json对象
		json_data = data.to_json();
		std::string json_string = json_data.dump();
#ifdef DEBUG
		printf("send: %s\n", json_string.c_str());
#endif // DEBUG		

		// 发送结果给客户端
		memset(buffer, '\0', core::MAX_LINE);
		sprintf(buffer, "%s", json_string.c_str());
		int send_n = send(fd, buffer, strlen(buffer), 0);
		printf("send num: %d\n", send_n);

		return 0;
	}

	/*
	* @brief 请求远程调用（客户端）
	*/
	int rpc_call_client(int fd, std::string func_name, int p1, int p2) {
		char buffer[core::MAX_LINE];  /*读缓冲区*/
		memset(buffer, '\0', core::MAX_LINE);

		// 构建Data对象
		RPCData data(func_name, p1, p2);

		// 从Data对象转为json对象
		nlohmann::json json_data = data.to_json();
		std::string json_string = json_data.dump();
		// 发送结果给客户端
		sprintf(buffer, "%s", json_string.c_str());
		send(fd, buffer, strlen(buffer), 0);

#ifdef DEBUG
		printf("send: %s\n", json_string.c_str());
#endif // DEBUG	

		// 从fd中读入RPC响应
		memset(buffer, '\0', core::MAX_LINE);
		if (recv(fd, buffer, core::MAX_LINE, 0) == -1)
		{
			printf("receive error\n");
		}

#ifdef DEBUG
		std::string buffer_s(buffer);
		printf("receive: %s\n", buffer_s.c_str());
#endif // DEBUG	

		// 从buffer转为json对象
		if (!nlohmann::json::accept(buffer)) {
			printf("receive json cannot be parsed.\n");
			return 0;
		}
		json_data = nlohmann::json::parse(buffer);
		// 从json对象转为Data对象
		data.from_json(json_data);

		return data.retval;
	}
};
