""" 
Gets files from the S3 bucket and puts them in the cache to
simulate the cached operation 
""" 

import requests

server_path = "http://s3.amazonaws.com/content.udacity-data.com"

for file_path in open("./workload.txt").read().splitlines():
	full_path = "%s%s" % (server_path, file_path)
	print "Getting: %s" % full_path
	response = requests.get(full_path)
	with open("./%s" % file_path, "wb") as fout:
		fout.write(response.content)

