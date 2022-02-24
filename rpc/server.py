from concurrent import futures
import logging
import usertaste_pb2
import usertaste_pb2_grpc
import grpc

files=[("/file1",223),("/file2",445),("/file3",667)]

class UserTasteServicer(usertaste_pb2_grpc.UserTasteServicer):
    def init(self,request,context):
        return usertaste_pb2.Stat(status=200)
    def greet(self,request,context):
        return usertaste_pb2.Stat(status=200)
    def listfile(self,request,context):
        for p,s in files:
            yield usertaste_pb2.File(path=p,size=s)

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    usertaste_pb2_grpc.add_UserTasteServicer_to_server(UserTasteServicer(),server)
    server.add_insecure_port('[::]:50051')
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    logging.basicConfig()
    serve()

