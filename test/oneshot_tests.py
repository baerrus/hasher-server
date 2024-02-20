import unittest
import sys, socket, time


class OneShotTestCase(unittest.TestCase):

 #   def __init__(self, host="localhost", port=8000):
 #       super(OneShotTestCase, self).__init__()

    def setUp(self):
        self.host = "localhost"
        self.port = 8000
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))

    def tearDown(self):
        self.sock.close()

    def test_fox1(self):
        content = "The quick brown fox jumps over the lazy dog\n"
        self.sock.sendall(content.encode())
        time.sleep(0.5)
        self.sock.shutdown(socket.SHUT_WR)

        reply = ""

        while True:
            data = self.sock.recv(1024)
            if not data:
                break
            reply += data.decode()

        self.assertEqual(reply, "9e107d9d372bb6826bd81d3542a419d6\n")


    def test_fox2(self):
        content = "The quick brown fox jumps over the lazy dog.\n"
        self.sock.sendall(content.encode())
        time.sleep(0.5)
        self.sock.shutdown(socket.SHUT_WR)

        reply = ""

        while True:
            data = self.sock.recv(1024)
            if not data:
                break
            reply += data.decode()

 
        self.assertEqual(reply, "e4d909c290d0fb1ca068ffaddf22cbd0\n")

    def test_empty(self):
        content = "\n"
        self.sock.sendall(content.encode())
        time.sleep(0.5)
        self.sock.shutdown(socket.SHUT_WR)

        reply = ""

        while True:
            data = self.sock.recv(1024)
            if not data:
                break
            reply += data.decode()

    #    print(reply)

        self.assertEqual(reply, "d41d8cd98f00b204e9800998ecf8427e\n")
        

    def test_onechar(self):
        content = "1\n"
        self.sock.sendall(content.encode())
        time.sleep(0.5)
        self.sock.shutdown(socket.SHUT_WR)

        reply = ""

        while True:
            data = self.sock.recv(1024)
            if not data:
                break
            reply += data.decode()

        self.assertEqual(reply, "c4ca4238a0b923820dcc509a6f75849b\n")

    def test_twochar(self):
        content = "12\n"
        self.sock.sendall(content.encode())
        time.sleep(0.5)
        self.sock.shutdown(socket.SHUT_WR)

        reply = ""

        while True:
            data = self.sock.recv(1024)
            if not data:
                break
            reply += data.decode()

        self.assertEqual(reply, "c20ad4d76fe97759aa27a0c99bff6710\n")

    def test_threechar(self):
        content = "123\n"
        self.sock.sendall(content.encode())
        time.sleep(0.5)
        self.sock.shutdown(socket.SHUT_WR)

        reply = ""

        while True:
            data = self.sock.recv(1024)
            if not data:
                break
            reply += data.decode()

        self.assertEqual(reply, "202cb962ac59075b964b07152d234b70\n")

    def test_fourchar(self):
        content = "1234\n"
        self.sock.sendall(content.encode())
        time.sleep(0.5)
        self.sock.shutdown(socket.SHUT_WR)

        reply = ""

        while True:
            data = self.sock.recv(1024)
            if not data:
                break
            reply += data.decode()

        self.assertEqual(reply, "81dc9bdb52d04dc20036dbd8313ed055\n")

if __name__ == '__main__':
   unittest.main()
