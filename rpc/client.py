import grpc
import usertaste_pb2
import usertaste_pb2_grpc
import logging

def run():
    channel = grpc.insecure_channel('localhost:50051')
    stub = usertaste_pb2_grpc.UserTasteStub(channel)
    response = stub.listfile(usertaste_pb2.Want(most_used_file="/home/yonde/Documents/cpro/text/kaden-channel/kaden-channel-5774093.txt",prefetch_num=10))
    for file in response:
        print(file.path)

if __name__ == '__main__':
    logging.basicConfig()
    run()

