import sys

py3flag = sys.version_info[0] >= 3

if py3flag:
    import http.server as httpserver
else:
    import BaseHTTPServer as httpserver

import time
import jedi


class Handler(httpserver.BaseHTTPRequestHandler):

    def _write_string(self, s):
        self.wfile.write(s.encode("utf-8"))

    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        self._write_string("ok path:{0}".format(self.path))

    def do_POST(self):
        time_start = time.time()
        print(self.headers)

        self.send_response(200)
        self.end_headers()
        print("path:", self.path)

        post_data = self.rfile.read(int(self.headers["content-length"]))
        if py3flag:
            post_data = str(post_data, "utf-8")
        post_data += self.run_complete(post_data)
        self.wfile.write(post_data.encode("utf-8"))
        print("jedi-server: done {0} s".format(time.time() - time_start))

    def run_complete(self, post_data):
        """exec code completion"""

        # parse post_data. format is:
        # line,col,filename\r\n
        # source...
        idx = post_data.find("\r\n")
        if idx == -1:
            print("jedi-server/complete: invalid post code=1")
            return ""
        params = post_data[:idx].split(",")
        if len(params) != 3:
            print("jedi-server/complete: invalid post code=2")
            return ""
        line, column, filename = params
        line, column = int(line), int(column)
        source = post_data[idx + 2:]
        print("jedi-server/complete: ", line, column, filename)

        # do completion
        def print_candidate(type_, typed_text, arguement):
            # row format: type$typed_text$arguemnt
            return "{0}${1}${2}\n".format(type_, typed_text, arguement)

        script = jedi.Script(source, line, column)
        completions = script.completions()
        # output compltion results
        result = ""
        for c in completions:
            if c.type in ["function", "class"]:
                if hasattr(c, "params"):  # c is callable
                    sig = "({0})".format(
                        ", ".join(map(lambda x: x.description, c.params)))
                    result += print_candidate(c.type, c.name, sig)
                else:
                    result += print_candidate(c.type, c.name, "")
            elif c.type in ["statement", "instance", "module", "import", "param"]:
                result += print_candidate(c.type, c.name, "")
            elif c.type == "keyword":
                pass  # ignore keyword
            else:
                print("unknown type:{0} name:{1}".format(c.type, c.name))
        return result


if __name__ == "__main__":
    port = 8080
    if len(sys.argv) == 2:
        port = int(sys.argv[1])
    httpserver.HTTPServer.allow_reuse_address = True
    server = httpserver.HTTPServer(("localhost", port), Handler)
    print("jedi-server: start")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
