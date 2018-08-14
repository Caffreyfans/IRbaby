from requests import post, get
import json
""" 测试IRext RESTful """

if __name__ == '__main__':
	""" 获取 id 和 token """
	url = "http://irext.net/irext-server/app/app_login"
	body = {"appKey":"4279187e58326959f1bc047f7900b4ee", "appSecret":"157f29992370f02043aca66893716be9", "appType":"2"}
	header_dict = {"Content-Type": "application/json"}
	response = post(url=url, data=json.dumps(body), headers=header_dict)
	result = bytes.decode(response.content)
	result = json.loads(result)
	app_id = result["entity"]["id"]
	app_token = result["entity"]["token"]
	print (json.dumps(result, sort_keys=True, indent=2))

	""" 获取家电类型 """
	url = 'http://irext.net/irext-server/indexing/list_categories'
	body = {"id":app_id, "token":app_token, "from":"0", "count":"10"}
	response = post(url=url, data=json.dumps(body), headers=header_dict)
	result = bytes.decode(response.content)
	result = json.loads(result)

	print (json.dumps(result, sort_keys=True, indent=2))

	""" 获取品牌列表 """
	url = "http://irext.net/irext-server/indexing/list_brands"
	body = {"id":app_id, "token":app_token, "categoryId":1, "from":"0", "count":"10"}
	response = post(url=url, data=json.dumps(body), headers=header_dict)
	result = bytes.decode(response.content)
	result = json.loads(result)
	print (json.dumps(result, sort_keys=True, indent=2))

	""" 遥控码索引列表 """
	url = "http://irext.net/irext-server/indexing/list_indexes"
	body = {"id":app_id, "token":app_token, "categoryId":1, "brandId":4, "from":"0", "count":"10"}
	response = post(url=url, data=json.dumps(body), headers=header_dict)
	result = bytes.decode(response.content)
	result = json.loads(result)
	print (json.dumps(result, sort_keys=True, indent=2))

	""" 下载遥控码 """
	url = "http://irext.net/irext-server/operation/download_bin"
	body = {"id":app_id, "token":app_token, "indexId":3580}
	response = post(url=url, data=json.dumps(body), headers=header_dict)
	#print (body)
	print ("try to download bin file>>>")
	if(len(response.content) == 0):
		print ("Http error code")
		print (response.status_code)	
	else:
		with open("irext.bin", 'wb') as fp:
			fp.write(response.content)
			fp.close()
			print ("下载 ok")
	