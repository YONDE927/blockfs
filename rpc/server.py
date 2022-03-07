from concurrent import futures
import logging
import usertaste_pb2
import usertaste_pb2_grpc
import grpc
import os,sys
import tfidf

files=[("/file1",223),("/file2",445),("/file3",667)]



class UserTasteServicer(usertaste_pb2_grpc.UserTasteServicer):
    def __init__(self):
        super().__init__()
        self.TSE = tfidf.TfidfSearchEngine()
        self.TSE.set_stop_pos(["記号","フィラー","その他","接頭詞","接続詞","助詞","助動詞"])
    def init(self,request,context):
        return usertaste_pb2.Stat(status=200)
    def greet(self,request,context):
        return usertaste_pb2.Stat(status=200)
    def listfile(self,request,context):
        directory = os.path.dirname(request.most_used_file)
        files = []
        for (dirpath, dirnames, filenames) in os.walk(directory):
            files.extend(filenames)
            break
        docs=[]
        entry=[]
        for file in files:
            if(".txt" in file):
                with open(directory + "/" + file,'r') as f:
                    docs.append(f.read())
                    entry.append(file)
        self.TSE.register(docs,entry)
        text=""
        with open(request.most_used_file,"r") as f:
            text = f.read()
        tmp = self.TSE.similardoc(text,size=request.prefetch_num)
        if(tmp):
            output = [directory + "/" + i for i,s in tmp]
        else:
            output = None
        if(output):
            for p in output:
                yield usertaste_pb2.File(path=p)

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    usertaste_pb2_grpc.add_UserTasteServicer_to_server(UserTasteServicer(),server)
    server.add_insecure_port('[::]:50051')
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    logging.basicConfig()
    serve()

