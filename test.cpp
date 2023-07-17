#include "httplib.h"

int main(){
    // HTTPS
    httplib::Server svr;

    std::cout << "Server started" << std::endl;

    svr.Post("/input", [](const httplib::Request &req, httplib::Response &res) {
        std::cout << req.get_param_value("sentence") << std::endl;
    });

    svr.listen("0.0.0.0", 8080);
}