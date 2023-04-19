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
	nlohmann::json params;
	nlohmann::json retval;

private:
	/*
	* @brief 变长参数模板递归拆包终止函数
	*/
	void parameterParse(nlohmann::json& j, int)
	{
		return;
	}

	/*
	* @brief 变长参数模板递归拆包函数
	*/
	template<typename T, typename... Ts>
	void parameterParse(nlohmann::json& j, int n, T first, Ts... args)
	{
		j[std::to_string(n).c_str()] = first;
		parameterParse(j, n + 1, args...);
	}

public:
	/*
	* @brief 默认构造函数
	*/
	RPCData() = default;

	/*
	* @brief 有参构造函数
	*/
	template<typename... Ts>
	RPCData(std::string _name, Ts&&... args) :
		protocol_name("RPC1.0"), function_name(_name) {
		param_number = sizeof...(args);

		//nlohmann::json json_params;
		parameterParse(params, 0, args...);

#ifdef DEBUG
		printf("param: %s\n", params.dump().c_str());
#endif
	}

	/*
	* @brief 对象转json
	*/
	nlohmann::json to_json() {
		nlohmann::json j = nlohmann::json{
			{"protocol_name", protocol_name},
			{"param_number", param_number},
			{"function_name", function_name},
			{"params", params},
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
		j.at("params").get_to(params);
		j.at("retval").get_to(retval);
	}
};

/*
* @brief 获取类型，默认为void赋予int类型
*/
template<typename T>
struct type_traits { typedef T type; };
template<>
struct type_traits<void> { typedef int type; };

/*
* @brief RPC类，用于处理RPC逻辑
*/
class RPCStub {
private:
	rpc_role_type role;  /*RPC角色*/
	std::unordered_map<std::string, std::function<void(RPCData&)>> stub_map;  /*本地存根*/

private:
	/*
	* @brief 函数调用代理，用于本地存根
	*/
	template<typename F>
	void call_proxy(F func, RPCData& data) {
		call_proxy_helper(func, data);
	}

	/*
	* @brief 普通函数调用拆包
	*/
	template<typename R>
	void call_proxy_helper(R(*func)(), RPCData& data) {
		call_proxy_helper(std::function<R()>(func), data);
	}
	template<typename R, typename P1>
	void call_proxy_helper(R(*func)(P1), RPCData& data) {
		call_proxy_helper(std::function<R(P1)>(func), data);
	}
	template<typename R, typename P1, typename P2>
	void call_proxy_helper(R(*func)(P1, P2), RPCData& data) {
		call_proxy_helper(std::function<R(P1, P2)>(func), data);
	}
	template<typename R, typename P1, typename P2, typename P3>
	void call_proxy_helper(R(*func)(P1, P2, P3), RPCData& data) {
		call_proxy_helper(std::function<R(P1, P2, P3)>(func), data);
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4>
	void call_proxy_helper(R(*func)(P1, P2, P3, P4), RPCData& data) {
		call_proxy_helper(std::function<R(P1, P2, P3, P4)>(func), data);
	}

	/*
	* @brief std::function调用拆包
	*/
	template<typename R>
	void call_proxy_helper(std::function<R()>(func), RPCData& data) {
		assert(data.param_number == 0);
		// 将参数bind到函数上，然后处理不同返回值的情况
		typename type_traits<R>::type retval = call_proxy_wrapper<R>(
			func);
		data.retval["value"] = retval;
	}
	template<typename R, typename P1>
	void call_proxy_helper(std::function<R(P1)>(func), RPCData& data) {
		assert(data.param_number == 1);
		P1 p1;
		data.params.at("0").get_to(p1);
		// 将参数bind到函数上，然后处理不同返回值的情况
		typename type_traits<R>::type retval = call_proxy_wrapper<R>(
			std::bind(func, p1));
		data.retval["value"] = retval;
	}
	template<typename R, typename P1, typename P2>
	void call_proxy_helper(std::function<R(P1, P2)>(func), RPCData& data) {
		assert(data.param_number == 2);
		P1 p1;
		P2 p2;
		data.params.at("0").get_to(p1);
		data.params.at("1").get_to(p2);
		// 将参数bind到函数上，然后处理不同返回值的情况
		typename type_traits<R>::type retval = call_proxy_wrapper<R>(
			std::bind(func, p1, p2));
		data.retval["value"] = retval;
	}
	template<typename R, typename P1, typename P2, typename P3>
	void call_proxy_helper(std::function<R(P1, P2, P3)>(func), RPCData& data) {
		assert(data.param_number == 3);
		P1 p1;
		P2 p2;
		P3 p3;
		data.params.at("0").get_to(p1);
		data.params.at("1").get_to(p2);
		data.params.at("2").get_to(p3);
		// 将参数bind到函数上，然后处理不同返回值的情况
		typename type_traits<R>::type retval = call_proxy_wrapper<R>(
			std::bind(func, p1, p2, p3));
		data.retval["value"] = retval;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4>
	void call_proxy_helper(std::function<R(P1, P2, P3, P4)>(func), RPCData& data) {
		assert(data.param_number == 4);
		P1 p1;
		P2 p2;
		P3 p3;
		P4 p4;
		data.params.at("0").get_to(p1);
		data.params.at("1").get_to(p2);
		data.params.at("2").get_to(p3);
		data.params.at("3").get_to(p4);
		// 将参数bind到函数上，然后处理不同返回值的情况
		typename type_traits<R>::type retval = call_proxy_wrapper<R>(
			std::bind(func, p1, p2, p3, p4));
		data.retval["value"] = retval;
	}

	/*
	* @brief 作为中间函数处理返回值为空的情况
	*/
	template<typename R, typename F>
	typename std::enable_if<std::is_same<R, void>::value, int>::type
		call_proxy_wrapper(F func) {
		func();
		return 0;
	}
	template<typename R, typename F>
	typename std::enable_if<!std::is_same<R, void>::value, R>::type
		call_proxy_wrapper(F func) {
		return func();
	}

public:
	/*
	* @brief 构造函数
	*/
	RPCStub(rpc_role_type _role) :role(_role) {}

public:
	/*
	* @brief 增加函数动态绑定到本地存根（服务器）
	*/
	template<typename F>
	void add_stub(std::string _name, F _func) {
		auto bind_func = std::bind(&RPCStub::call_proxy<F>, this, _func, std::placeholders::_1);
		stub_map[_name] = bind_func;
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
		auto callback = stub_map[data.function_name];
		callback(data);

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
	template<typename R, typename ...Ts>
	typename type_traits<R>::type rpc_call_client(int fd, std::string func_name, Ts... args) {
		char buffer[core::MAX_LINE];  /*读缓冲区*/
		memset(buffer, '\0', core::MAX_LINE);

		// 构建Data对象
		RPCData data(func_name, args...);

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

		typename type_traits<R>::type retval;
		data.retval.at("value").get_to(retval);
		return retval;
	}
};
