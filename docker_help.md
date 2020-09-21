# Build docker
cd ~/Programs/dmp
docker build --network host --tag=abs_interp -f ../abs_interp/Dockerfile .

# Run Docker
docker run --name abs_interp -it --network host -v /home/divyanjali/Programs/dmp:/home/apron/mp -v /home/divyanjali/Programs/abs_interp:/home/abs_interp abs_interp
- ```cpus=5``` : use 5 cpus
- ```m=10g``` : use 10GB memory 

# Running the program
```
export CPLUS_INCLUDE_PATH=/tmp/include/
export LIBRARY_PATH=/tmp/lib/
export LD_LIBRARY_PATH=/tmp/lib/
mkdir /home/divyanjali/poet-build
cd /home/divyanjali/poet-build
cmake ../../abs_interp/
make
opt -load interp/VerifierPass.so -verifier -interval -stop-on-fail ../../abs_interp/tests/litmus/test35.ll -no-interf-comb
```

# Other Docker related commands
## List docker container/image
docker container ls -a
docker image ls -a

## Removing docker container/image
docker container rm abs_interp
docker image rm abs_interp

## Start docker


## Stop docker

