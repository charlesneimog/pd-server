//
//  sample.cc
//
//  Copyright (c) 2022. Charles K. Neimog
//  MIT License
//

#include <chrono>
#include <cstdio>

#include "./cpp-httplib/httplib.h" 
#include <m_pd.h>
#include <g_canvas.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

// include JSON library
#include "./json/single_include/nlohmann/json.hpp"


// WebRTC
// #include "./libdatachannel/include/rtc/rtc.hpp"
// using namespace rtc;
//
// #include "./libdatachannel/include/rtc/rtc.hpp"
// #include "libdatachannel/include/rtc/configuration.hpp"
// using namespace rtc;
//
//
//
// static void onDataChannelMessage(const rtc::DataChannel &channel, const rtc::Message &msg) {
//     if (msg.type == rtc::Message::Binary) {
//         const float* audio_data = reinterpret_cast<const float*>(msg.data());
//         size_t num_samples = msg.size() / sizeof(float);
//
//         // Print the first 20 samples
//         for (size_t i = 0; i < std::min(num_samples, static_cast<size_t>(20)); i++) {
//             post("Audio sample %d: %f", i, audio_data[i]);
//         }
//     }
// }
//
//
// int mainTest() {
//     rtc::Configuration rtcConfig;
//     rtcConfig.iceServers.emplace_back(rtc::IceServer{
//         "stun:192.168.8.15:1998"
//
//     });
//     rtc::PeerConnection pc(rtcConfig);
//
//     rtc::Configuration dcConfig;
//
//
//     std::shared_ptr<rtc::DataChannel> dc;
//
//     dc = pc.createDataChannel("audio");
//     
//     dc->onOpen([]() {
//         post("DataChannel open");
//     });
//
//     dc->onMessage([](std::variant<rtc::binary, rtc::string> message) {
//         if (std::holds_alternative<rtc::string>(message)) {
//             post("Received string: %s", std::get<rtc::string>(message).c_str());
//         }
//     });
//
//     return 0;
// }




// ==========================
t_class *server_class;
using namespace httplib;
using json = nlohmann::json;


// DEFINE GLOBAL VARIABLE TO SAVE THE SERVER SSLServer
SSLServer *GLOBAL_SERVER_HTTPS; // Temporary Resolution
Server *GLOBAL_SERVER_HTTP; // Temporary Resolution

// For warning messages when creating more than one server object (seems unstable, mainly on Windows)
int server_object = 0;
int object_created = 0;

// =======================================================

typedef struct _server { // It seems that all the objects are some kind of class.
    t_object            x_obj; // convensao no puredata source code
	t_outlet 			*outlet; // outlet
	t_outlet            *out_audio; // outlet for audio
    t_canvas            *x_canvas; // pointer to the canvas
    t_symbol            *folderOfServer; // folder of the server
    t_float             audio; // audio
	t_int             	port; // port
	t_int  				running; // running
	t_int  				ssl; // ssl
	t_int 				debug;
	t_int 				post;
    t_int               audioRunning;
}t_server;

// ========================================================
static void server_free(t_server *x){
	(void)x;
	object_created -= 1;
	if (object_created == 0) {
		server_object = 0;
	}
}

// ========================================================

static void *server_new(t_symbol *s, int argc, t_atom *argv){
    (void)s;
	if (server_object == 0) {
		post("");
		post("[server] Server object is an interface to the httplib library by Yuji Hirose");
		post("[server] The interface for PureData is developed by Charles K. Neimog (2022)");
		post("[server] version 0.1.1");
		post("");
	}
	t_server *x = (t_server *)pd_new(server_class);
	if (server_object == 1){
		post("");
		pd_error(x, "[server] There is more than one server object, check if the port, same port cause problems");
		post("");
	}

    x->outlet = outlet_new(&x->x_obj, 0);
	x->debug = 0;
	x->port = 8080;
    x->folderOfServer = gensym("/public");
	if (argc != 0) {
		for (int i = 0; i < argc; i++) {
			if (argv[i].a_type == A_FLOAT) {
				x->port = (int)argv[i].a_w.w_float;
				post("[server] Port set to %d", x->port);
			}
			else if (argv[i].a_type == A_SYMBOL) {
				// check if symbol is equal "-audio"
				if (strcmp(argv[i].a_w.w_symbol->s_name, "-audio") == 0) {
					post("[server] Audio activated");
					x->out_audio = outlet_new(&x->x_obj, gensym("signal"));
				}
			}
		}
	}
	object_created += 1;
	x->x_canvas = canvas_getcurrent();
	std::string public_dir;
	t_symbol *canvas = canvas_getdir(x->x_canvas);
	public_dir = canvas->s_name;
	x->running = 0;
	server_object = 1;
	return(x);
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


// // ====================================
// // Function to handle incoming audio data
// void on_message_binary(t_server *x, std::shared_ptr<WebSocketServer::Connection> connection,
//                        std::shared_ptr<WebSocketServer::InMessage> message) {
//     auto buffer = message->string();
//     auto data = reinterpret_cast<const int16_t*>(buffer.c_str());
//     size_t data_size = buffer.size() / sizeof(int16_t);
//
//     std::vector<int16_t> audio_data(data, data + data_size);
//     
//     // Add the audio data to the synchronized queue
//     audio_data_queue.push(audio_data);
// }
//
// // ====================================
// static void receive_audio(t_server *x, t_floatarg f) {
//     (void)x;
//     (void)f;
//
//     // get complete path to the certificate and key
//     std::string public_dir;
// 	std::string server_crt;
// 	std::string server_key;
//     t_symbol *canvas = canvas_getdir(x->x_canvas);
// 	public_dir = canvas->s_name;
//
// 	server_crt = public_dir + "/server.crt";
// 	server_key = public_dir + "/server.key";
//     
//     WebSocketServer server;
//     server.config.port = 8001;
//     server.config.address = get_ip_address(x);
//     auto &audioReceiver = server.endpoint["^/audio/?$"];
//
//     auto on_message = [x](std::shared_ptr<WebSocketServer::Connection> connection, std::shared_ptr<WebSocketServer::InMessage> message) {
//         on_message_binary(x, connection, message);
//     };
//
//     audioReceiver.on_error = [](shared_ptr<WebSocketServer::Connection> connection, const SimpleWeb::error_code &ec) {
//         pd_error(NULL, "[server] Audio error: %s", ec.message().c_str());
//     };
//
//     audioReceiver.on_message = on_message;
//     // audioReceiver.on_error = on_error;
//
//     // another things
//     audioReceiver.on_open = [](std::shared_ptr<WebSocketServer::Connection> connection) {
//         post("[server] Audio connection opened");
//     };
//
//     audioReceiver.on_close = [](std::shared_ptr<WebSocketServer::Connection> connection, int status, const std::string &reason) {
//         post("[server] Audio connection closed");
//     };
//     
//     server.start();
//     
//
//     return;
//
// }
//
// // ========================================================
// static void receiveaudio(t_server *x, t_floatarg f) {
//     (void)x;
//     (void)f;
//     if (x->audioRunning == 1) {
//         post("[server] Audio already running");
//         return;
//     }
//     x->audioRunning = 1;
//     std::string ip_address = get_ip_address(x);
// 	int port = x->port;
// 	post("[server] Audio started on wss://%s:8001", ip_address.c_str());
//     std::thread(receive_audio, x, f).detach();
//         
// }
//
//
//
// // ========================================================
// t_int *server_perform(t_int *w) {
//     t_server *x = (t_server *)(w[1]);
//     
//     int sig_len = (int)(w[3]);
//
//     if (!audio_data_queue.empty()) {
//         std::vector<int16_t> audio_data = audio_data_queue.pop();
//         t_float audio[audio_data.size()];
//         int audio_size = audio_data.size();
//         if (audio_size == sig_len){
//             t_sample *audioOut = (t_sample *)(w[2]);
//             for (int i = 0; i < audio_size; i++) {
//                 audio[i] = (t_float)audio_data[i];
//                 audioOut[i] = audio[i] / 32768.0;
//             }
//             // clear the audio_data_queue
//             // audio_data_queue.clear();
//
//         }
//         else{
//             pd_error(x, "[server] Audio size is different from signal size, use switch~ to change the size");
//             return (w + 4);
//         }
//     }
//     else{
//         logpost(x, 3, "[server] No audio data in the queue");
//     }
//     
//     return (w + 4);  // Return the pointer to the next object
// }
//
// // ========================================================
// static void server_out(t_server *x, t_signal **sp){
//     post("server_perform activated");
//     dsp_add(server_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
//     return;
// }


// ========================================================
static void set_port(t_server *x, t_floatarg f) {
	x->port = (int)f;
	post("[server] Port set to %d", x->port);
	return;
}

// ========================================================
static void set_folder(t_server *x, t_symbol *s) {
    x->folderOfServer = s;
    post("[server] Folder set to %s", x->folderOfServer->s_name);
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
static void get_ip(t_server *x) {
    // get_ip_address
    std::string ip_address = get_ip_address(x);
    post("[server] IP address: %s", ip_address.c_str());
    outlet_anything(x->outlet, gensym(ip_address.c_str()), 0, NULL);
    return;

    
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

	public_dir += x->folderOfServer->s_name;
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

    public_dir += x->folderOfServer->s_name;
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
							A_GIMME, 
							0);
  	
	// METHODS
	class_addmethod(server_class, (t_method)start_server, gensym("start"), A_NULL, 0);
	class_addmethod(server_class, (t_method)stop_server, gensym("stop"), A_NULL, 0);
	class_addmethod(server_class, (t_method)set_port, gensym("port"), A_FLOAT, 0);
	class_addmethod(server_class, (t_method)set_ssl, gensym("ssl"), A_FLOAT, 0);
	class_addmethod(server_class, (t_method)debug, gensym("debug"), A_FLOAT, 0);
	class_addmethod(server_class, (t_method)set_post, gensym("post"), A_FLOAT, 0);
    class_addmethod(server_class, (t_method)get_ip, gensym("getip"), A_NULL, 0);
    class_addmethod(server_class, (t_method)set_folder, gensym("folder"), A_SYMBOL, 0);
    // class_addmethod(server_class, (t_method)receiveaudio, gensym("audio"), A_FLOAT, 0);

    // AUDIO
    // CLASS_MAINSIGNALIN(server_class, t_server, audio);
    // class_addmethod(server_class, (t_method)server_out, gensym("dsp"), A_CANT, 0);

}
