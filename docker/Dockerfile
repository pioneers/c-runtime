FROM pierobotics/runtime:base

WORKDIR /root/runtime

RUN apt-get update && apt-get install -y openssh-server
RUN service ssh start

RUN useradd -m -s /bin/bash -p $(openssl passwd runtime) runtime
RUN usermod -aG sudo runtime

# Adds all files in the repo that are not in .dockerignore
ADD ./ ./

# Add the runtime command to our PATH
RUN echo 'export PATH=$PATH:~/runtime' >> ~/.bashrc

# Build Runtime processes
RUN ./runtime build

# By default, run Runtime when the Docker container starts
CMD ./runtime run
