int main (int argc, char** argv)
{
	//wait for connection and HTTP GET request (you may do this single threaded or multi-threaded)

	//After receiving GET request, open file requested and send with HTTP 200 OK code

	//	If file requested does not exist, return 404 Not Found code with custom File Not Found page
	//	If HTTP request is for SecretFile.html, return 401 Unauthorized
	//	If request is for file that is above the directory structure where web server is running ( for example, "GET ../../../etc/passwd" ), return 403 Forbidden
	//	if server cannot understand request return 400 Bad Request
	
	//After handling request, return to waiting for next request.
}