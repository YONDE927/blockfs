import grpc
import usertaste_pb2
import usertaste_pb2_grpc
import logging

def run():
    channel = grpc.insecure_channel('localhost:50051')
    stub = usertaste_pb2_grpc.UserTasteStub(channel)
    response = stub.greet(usertaste_pb2.Want(something="active",size=0))
    print(response.status)

if __name__ == '__main__':
    logging.basicConfig()
    run()

