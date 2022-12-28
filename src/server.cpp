//
//  sample.cc
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//
#include <chrono>
#include <cstdio>
#include "httplib.h"  
#include <m_pd.h>
#include <g_canvas.h>
#include <iostream>
#include <pthread.h>

t_class *server_class;
using namespace httplib;

// ========================================================
typedef struct _server { // It seems that all the objects are some kind of class.
    t_object            x_obj; // convensao no puredata source code
    t_canvas            *x_canvas; // pointer to the canvas
    t_inlet             *in1;
	t_float             *port; // port
	t_float			 	*https; // https
	t_symbol 			*cert_file;
	t_symbol 			*private_key_file;
}t_server;

// ========================================================

void *server_new(void) {
	post("");
	post("[server] The server is an interface to the httplib library");
	post("[server] httplib is developed by Yuji Hirose (MIT License)"); 
	post("[server] Check the original code in: https://github.com/yhirose/cpp-httplib/");
	post("[server] Pd server object developed by Charles K. Neimog (2022)");
	post("[server] version 0.0.1");
	post("");
	t_server *x = (t_server *)pd_new(server_class);
	x->x_canvas = canvas_getcurrent();
	std::string public_dir;
	t_symbol *canvas = canvas_getdir(x->x_canvas);
	public_dir = canvas->s_name;
	x->port = (t_float *)malloc(sizeof(t_float));
	*x->port = 8080;
	return (void *)x;
}

// ========================================================
static void server_free(t_server *x) {
	free(x->port);
}

// ========================================================
// ========================================================
static void set_port(t_server *x, t_floatarg f) {
	*x->port = f;
	int port = *x->port;
	post("[server] Server port set to %d", port);
	return;
}

// ========================================================
// ========================================================

static void server_main(t_server *x) {
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

	// start server
	#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  		SSLServer svr(SERVER_CERT_FILE, SERVER_PRIVATE_KEY_FILE);
	#else
  		Server svr;
	#endif
	
	if (!svr.is_valid()) {
		pd_error(x, "[server] Server could not be started");
		return;
	} 

	public_dir += "/public";
	svr.set_mount_point("/", public_dir.c_str()); // all must be inside a public folder
	svr.set_error_handler([](const Request & /*req*/, Response &res) {
		char buf[BUFSIZ];
			res.set_content(buf, "text/html");
	});
	svr.Get("/", [](const Request & /*req*/, Response &res) {
			res.set_redirect("/index.html");
	});


	// ========================================================
	int port = *x->port;
	post("[server] Server started on port %d", port);
	
	svr.listen("0.0.0.0", port);
	return;
}

// ========================================================
static void *stop_server(t_server *x) {
	post("[server] Not implemented yet");
	return NULL;

}


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
	server_main(&arg->x);
	free(arg);
	return NULL;

}

// ========================================================
static void start_server(t_server *x, t_symbol *s, int argc, t_atom *argv) {
	struct thread_arg_struct *arg = (struct thread_arg_struct *)malloc(sizeof(struct thread_arg_struct));
    arg->x = *x;
	pthread_t thread;
	pthread_create(&thread, NULL, start_server_thread, arg);
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
  	server_class = class_new(gensym("server"), server_new, 0, sizeof(t_object), 0, A_NULL);
  	
	// METHODS
	class_addmethod(server_class, (t_method)start_server, gensym("start"), A_GIMME, 0);
	class_addmethod(server_class, (t_method)stop_server, gensym("stop"), A_NULL, 0);
	class_addmethod(server_class, (t_method)set_port, gensym("port"), A_FLOAT, 0);
}

// ========================================================