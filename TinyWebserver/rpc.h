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
	nlohmann::json params;
	nlohmann::json retval;

private:
	/*
	* @brief �䳤����ģ��ݹ�����ֹ����
	*/
	void parameterParse(nlohmann::json& j, int)
	{
		return;
	}

	/*
	* @brief �䳤����ģ��ݹ�������
	*/
	template<typename T, typename... Ts>
	void parameterParse(nlohmann::json& j, int n, T first, Ts... args)
	{
		j[std::to_string(n).c_str()] = first;
		parameterParse(j, n + 1, args...);
	}

public:
	/*
	* @brief Ĭ�Ϲ��캯��
	*/
	RPCData() = default;

	/*
	* @brief �вι��캯��
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
	* @brief ����תjson
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
	* @brief jsonת����
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
* @brief ��ȡ���ͣ�Ĭ��Ϊvoid����int����
*/
template<typename T>
struct type_traits { typedef T type; };
template<>
struct type_traits<void> { typedef int type; };

/*
* @brief RPC�࣬���ڴ���RPC�߼�
*/
class RPCStub {
private:
	rpc_role_type role;  /*RPC��ɫ*/
	std::unordered_map<std::string, std::function<void(RPCData&)>> stub_map;  /*���ش��*/

private:
	/*
	* @brief �������ô������ڱ��ش��
	*/
	template<typename F>
	void call_proxy(F func, RPCData& data) {
		call_proxy_helper(func, data);
	}

	/*
	* @brief ��ͨ�������ò��
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
	* @brief std::function���ò��
	*/
	template<typename R>
	void call_proxy_helper(std::function<R()>(func), RPCData& data) {
		assert(data.param_number == 0);
		// ������bind�������ϣ�Ȼ����ͬ����ֵ�����
		typename type_traits<R>::type retval = call_proxy_wrapper<R>(
			func);
		data.retval["value"] = retval;
	}
	template<typename R, typename P1>
	void call_proxy_helper(std::function<R(P1)>(func), RPCData& data) {
		assert(data.param_number == 1);
		P1 p1;
		data.params.at("0").get_to(p1);
		// ������bind�������ϣ�Ȼ����ͬ����ֵ�����
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
		// ������bind�������ϣ�Ȼ����ͬ����ֵ�����
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
		// ������bind�������ϣ�Ȼ����ͬ����ֵ�����
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
		// ������bind�������ϣ�Ȼ����ͬ����ֵ�����
		typename type_traits<R>::type retval = call_proxy_wrapper<R>(
			std::bind(func, p1, p2, p3, p4));
		data.retval["value"] = retval;
	}

	/*
	* @brief ��Ϊ�м亯��������ֵΪ�յ����
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
	* @brief ���캯��
	*/
	RPCStub(rpc_role_type _role) :role(_role) {}

public:
	/*
	* @brief ���Ӻ�����̬�󶨵����ش������������
	*/
	template<typename F>
	void add_stub(std::string _name, F _func) {
		auto bind_func = std::bind(&RPCStub::call_proxy<F>, this, _func, std::placeholders::_1);
		stub_map[_name] = bind_func;
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
		auto callback = stub_map[data.function_name];
		callback(data);

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
	template<typename R, typename ...Ts>
	typename type_traits<R>::type rpc_call_client(int fd, std::string func_name, Ts... args) {
		char buffer[core::MAX_LINE];  /*��������*/
		memset(buffer, '\0', core::MAX_LINE);

		// ����Data����
		RPCData data(func_name, args...);

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

		typename type_traits<R>::type retval;
		data.retval.at("value").get_to(retval);
		return retval;
	}
};
