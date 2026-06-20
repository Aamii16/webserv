#include "core_server.hpp"
#include <cstdlib>

int main(void)
{

    t_server cfg;
    cfg.host          = "0.0.0.0";
    cfg.port          = 8080;
    cfg.server_name   = "localhost";
    cfg.root          = "./www";
    cfg.max_body_size = 1048576;

    location root_loc;
    root_loc.methods[GET]    = true;
    root_loc.methods[POST]   = true;
    root_loc.methods[DELETE] = false;
    root_loc.root            = "./www";
    cfg.locations["/"]       = root_loc;

    CoreServer core;
    core.addServer(cfg.host, cfg.port, cfg);
    core.run();

    return EXIT_SUCCESS;
}
