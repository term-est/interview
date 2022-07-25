A simple Server/Client application.

Developed as part of the ANAYURT interview process. A fully async server/client application with a robust and extendable platform inddependent framework written with modern C++ that is just shy of 800 loc

# Dependencies

- asio
- ftxui ( for TUI which doesn't exist yet ;_; )
- spdlog 
- fmt
- pugixml ( why yes, I am using xml as a database, how can you tell? )
- argparse

CMake targets Server and Client are both executables and not libraries. Just build it and you are good to go!
It uses localhost loopback and this is hardcoded, but you can give a -port arg to specify which port you want to run the server on.


# TODO

- An actual TUI would be nice
- Client connect is not async as of now
- Refactor and cleanup
- ReceiveCallback takes an `auto`, a nice concept would be nice for type safety

### Demo
[![Demo](https://img.youtube.com/vi/l6bR-skqx4o/sddefault.jpg)](https://youtu.be/l6bR-skqx4o "Demo")
