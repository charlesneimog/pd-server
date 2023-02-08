//
//  sample.cc
//
//  Copyright (c) 2022. Charles K. Neimog
//  MIT License
//

#include <chrono>
#include <cstdio>
#include <stdlib.h>

#include "./cpp-httplib/httplib.h" 
#include <m_pd.h>
#include <g_canvas.h>
#include <iostream>
#include <pthread.h>

// include JSON library
#include "./json/single_include/nlohmann/json.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

t_class *server_class;
using namespace httplib;
using json = nlohmann::json;

// DEFINE GLOBAL VARIABLE TO SAVE THE SERVER SSLServer
SSLServer *GLOBAL_SERVER_HTTPS; // Temporary Resolution
Server *GLOBAL_SERVER_HTTP; // Temporary Resolution

// For warning messages when creating more than one server object (seems unstable, mainly on Windows)
int server_object = 0;
int object_created = 0;

// ========================================================
typedef struct _server { // It seems that all the objects are some kind of class.
    t_object            x_obj; // convensao no puredata source code
	t_outlet 			*outlet; // outlet
    t_canvas            *x_canvas; // pointer to the canvas
	t_int             	port; // port
	t_int  				running; // running
	t_int  				ssl; // ssl
	t_int 				debug;
	t_int 				post;
}t_server;

// ========================================================

static void *server_new(t_floatarg f) {
	if (server_object == 0) {
		post("");
		post("[server] Server object is an interface to the httplib library by Yuji Hirose");
		post("[server] The interface for PureData is developed by Charles K. Neimog (2022)");
		post("[server] version 0.1.0");
		post("");
	}
	t_server *x = (t_server *)pd_new(server_class);
	if (server_object == 1){
		post("");
		pd_error(x, "[server] It is not recommended to create more than one server object");
		post("");
	}

	x->debug = 0;
	x->port = 8080;
	if (f != 0) {
		x->port = (int)f;
		post("[server] Port set to %d", x->port);
	}

	object_created += 1;
	x->x_canvas = canvas_getcurrent();
	std::string public_dir;
	t_symbol *canvas = canvas_getdir(x->x_canvas);
	public_dir = canvas->s_name;
	x->running = 0;
	server_object = 1;
	x->outlet = outlet_new(&x->x_obj, 0);
	return(x);
}

// ========================================================
static void server_free(t_server *x){
	(void)x;
	object_created -= 1;
	if (object_created == 0) {
		server_object = 0;
	}
}

// ========================================================
static void set_port(t_server *x, t_floatarg f) {
	// convert to int
	x->port = (int)f;
	post("[server] Port set to %d", x->port);
	return;
}

// ========================================================
static void set_ssl(t_server *x, t_floatarg f) {
	// convert to int
	x->ssl = (int)f;
	if (x->ssl == 1) {
		post("[server] SSL activated");
	} else {
		post("[server] SSL deactivated");
	}
	return;
}

// ========================================================
static void debug(t_server *x, t_floatarg f) {
	// convert to int
	int debug = (int)f;
	if (debug == 1) {
		post("[server] Debug activated");
		x->debug = 1;
	} else {
		post("[server] Debug deactivated");
		x->debug = 0;
	}
	return;
}

// ========================================================
static void set_post(t_server *x, t_floatarg f) {
	// convert to int
	int post_value = (int)f;
	if (post_value == 1) {
		post("[server] Post activated");
		x->post = 1;
		// create outlet

	} else {
		post("[server] Post deactivated");
		x->post = 0;
	}
	return;
}

// ===========================================================:
static void received_message(t_server *x, std::string message) {
	json j = json::parse(message); // this is from the nlohmann json library
	// get all keys members of the json object
	for (json::iterator it = j.begin(); it != j.end(); ++it) {
		// output the key and the value
		std::string key = it.key();
		if (it.value().is_string()) {
			t_atom a;
			SETSYMBOL(&a, gensym(it.value().get<std::string>().c_str()));
			outlet_anything(x->outlet, gensym(key.c_str()), 1, &a);
		} else if (it.value().is_number()) {
			int value = it.value();
			t_atom a;
			SETFLOAT(&a, value);
			outlet_anything(x->outlet, gensym(key.c_str()), 1, &a);
		} else if (it.value().is_array())
		{
			json array = it.value();
			int size = array.size();
			t_atom a[size];
			for (int i = 0; i < size; i++) {
				if (array[i].is_string()) {
					SETSYMBOL(&a[i], gensym(array[i].get<std::string>().c_str()));
				} else if (array[i].is_number()) {
					int value = array[i];
					SETFLOAT(&a[i], value);
				} else {
					pd_error(x, "[server] %s is not a string or a number", key.c_str());
				}
			}
			outlet_anything(x->outlet, gensym(key.c_str()), size, a);
		}
		else {
			pd_error(x, "[server] %s is not a string, number, or list.", key.c_str());
		}
	}
	return;
}

// ========================================================
static std::string get_ip_address(t_server *x) {
	std::string ip_address;
	#ifdef _WIN32
		char ac[80];
		if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
			pd_error(x, "[server] Error in gethostname");
			return NULL;
		}
		struct hostent *phe = gethostbyname(ac);
		if (phe == 0) {
			pd_error(x, "[server] Error in gethostbyname");
			return NULL;
		}
		for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
			ip_address = inet_ntoa(addr);
		}
	#else
		struct ifaddrs *ifaddr, *ifa;
		int family, s;
		char host[NI_MAXHOST];
		if (getifaddrs(&ifaddr) == -1) {
			pd_error(x, "[server] Error in getifaddrs");
			return NULL;
		}
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr == NULL)
				continue;
			family = ifa->ifa_addr->sa_family;
			if (family == AF_INET) {
				s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if (s != 0) {
					pd_error(x, "[server] Error in getnameinfo");
					return NULL;
				}
				ip_address = host;
			}
		}
		freeifaddrs(ifaddr);
	#endif
	return ip_address;
}

// ========================================================
static void server_https(t_server *x) {
	std::string public_dir;
	std::string cert_path;
	std::string private_key_path;
	t_symbol *canvas = canvas_getdir(x->x_canvas);
	public_dir = canvas->s_name;

	cert_path = public_dir + "/cert.pem";
	private_key_path = public_dir + "/key.pem";

	if (access(cert_path.c_str(), F_OK) == -1) {
		pd_error(x, "[server] Cert file not found");
		return;
	}
	if (access(private_key_path.c_str(), F_OK) == -1) {
		pd_error(x, "[server] Private key file not found");
		return;
	}
	
	const char *cert_path_char = cert_path.c_str();
	const char *private_key_path_char = private_key_path.c_str();
	
	#define SERVER_CERT_FILE cert_path_char
	#define SERVER_PRIVATE_KEY_FILE private_key_path_char

	SSLServer svr(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
	
	if (!svr.is_valid()) {
		pd_error(x, "[server] Server could not be started");
		return;
	} 

	// save the server in the GLOBAL_SERVER variable
	GLOBAL_SERVER_HTTPS = &svr;

	public_dir += "/public";
	svr.set_mount_point("/", public_dir.c_str()); // all must be inside a public folder
	svr.set_error_handler([](const Request & /*req*/, Response &res) {
		char buf[BUFSIZ];
			res.set_content(buf, "text/html");
	});
	svr.Get("/", [](const Request & /*req*/, Response &res) {
			res.set_redirect("/index.html");
	});

	// Adress to stop the server
	svr.Get("/stop", [&](const Request & /*req*/, Response &res) {
		svr.stop();
		x->running = 0;
	});
	
	// get post message from send2pd
	svr.Post("/send2pd", [&](const Request &req, Response &res) {
		std::string message = req.body;
		// message is a json string, we need to parse it
		received_message(x, message);
		res.set_content("", "text/plain");
	});

	std::string ip_address = get_ip_address(x);
	int port = x->port;
	post("[server] Server started on https://%s:%d", ip_address.c_str(), port);
	svr.listen("0.0.0.0", port);
	post("[server] Server stopped");
	return;
}

// ========================================================
static void server_http(t_server *x) {
	std::string public_dir;
	std::string cert_path;
	std::string private_key_path;
	t_symbol *canvas = canvas_getdir(x->x_canvas);
	public_dir = canvas->s_name;

	Server svr;
	
	if (!svr.is_valid()) {
		pd_error(x, "[server] Server could not be started");
		return;
	} 

	// save the server in the GLOBAL_SERVER variable
	GLOBAL_SERVER_HTTP = &svr;

	public_dir += "/public";
	svr.set_mount_point("/", public_dir.c_str()); // all must be inside a public folder
	svr.set_error_handler([](const Request & /*req*/, Response &res) {
		char buf[BUFSIZ];
			res.set_content(buf, "text/html");
	});
	svr.Get("/", [](const Request & /*req*/, Response &res) {
			res.set_redirect("/index.html");
	});

	// Adress to stop the server
	svr.Get("/stop", [&](const Request & /*req*/, Response &res) {
		svr.stop();
		x->running = 0;
	});	

	// get post message from send2pd
	svr.Post("/send2pd", [&](const Request &req, Response &res) {
		std::string message = req.body;
		// message is a json string, we need to parse it
		received_message(x, message);
		res.set_content("", "text/plain");
	});


	std::string ip_address = get_ip_address(x);
	int port = x->port;
	post("[server] Server started on http://%s:%d", ip_address.c_str(), port);
	svr.listen("0.0.0.0", port);
	post("[server] Server stopped");
	return;
}

// ========================================================
static void *stop_server(t_server *x) {
	if (x->running == 0) {
		pd_error(x, "[server] Server not running");
		return NULL;
	}
	if (x->ssl == 1){
		SSLServer *svr = GLOBAL_SERVER_HTTPS; // Temporary Resolution
		svr->stop();
	} else {
		Server *svr = GLOBAL_SERVER_HTTP; // Temporary Resolution
		svr->stop();
	}
	x->running = 0;
	return NULL;
}

// ========================================================
// make a Push Request. 





// ========================================================
struct thread_arg_struct {
    t_server x;
    t_symbol s;
    int argc;
    t_atom *argv;
} thread_arg;

// ========================================================
static void *start_server_thread(void *lpParameter) {
	thread_arg_struct *arg = (thread_arg_struct *)lpParameter;
	int ssl = arg->x.ssl;
	if (ssl == 1){
		server_https(&arg->x);
	} else {
		server_http(&arg->x);
	}
	free(arg);
	return NULL;
}

// ========================================================
static void start_server(t_server *x) {
	if (x->running == 1) {
		pd_error(x, "[server] Server already running");
		return;
	}
	struct thread_arg_struct *arg = (struct thread_arg_struct *)malloc(sizeof(struct thread_arg_struct));
    arg->x = *x;
	pthread_t thread;
	pthread_create(&thread, NULL, start_server_thread, arg);
	pthread_detach(thread);
	x->running = 1;
	return;
}

// ========================================================
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
	void server_setup(void);
}
#endif

// ========================================================
void server_setup(void) {
	std::cerr << __FUNCTION__ << std::endl;
  	server_class = class_new(gensym("server"), 
							(t_newmethod)server_new,
							(t_method)server_free, 
							sizeof(t_server), 
							CLASS_DEFAULT, 
							A_FLOAT, 
							0);
  	
	// METHODS
	class_addmethod(server_class, (t_method)start_server, gensym("start"), A_NULL, 0);
	class_addmethod(server_class, (t_method)stop_server, gensym("stop"), A_NULL, 0);
	class_addmethod(server_class, (t_method)set_port, gensym("port"), A_FLOAT, 0);
	class_addmethod(server_class, (t_method)set_ssl, gensym("ssl"), A_FLOAT, 0);
	class_addmethod(server_class, (t_method)debug, gensym("debug"), A_FLOAT, 0);
	class_addmethod(server_class, (t_method)set_post, gensym("post"), A_FLOAT, 0);
}
