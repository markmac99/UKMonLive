#define _CRT_SECURE_NO_WARNINGS

/*
Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
This file is licensed under the Apache License, Version 2.0 (the "License").
You may not use this file except in compliance with the License. A copy of
the License is located at
http://aws.amazon.com/apache2.0/
This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/
#include "UKMonLiveCL.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>

/**
* Put an object to an Amazon S3 bucket.
*/
int put_file(char* buckname, const char* fname, char* reg, char* acct, char* secret, long frcount, long maxbmax)
{
	Aws::SDKOptions options;
	Aws::InitAPI(options);
	{
		Aws::String file_name = ProcessingPath;
		file_name += "\\";
		file_name += fname;
		if (Debug) std::cout << "B1: " << fname << " " << buckname << std::endl;
		if (Debug) std::cout << "B2: " << file_name << std::endl;

		// filename may contain path relative to the processing path - we need to remove this before
		// fixing the key_name
		Aws::String kn = fname, key_name;
		size_t n = kn.find_last_of("\\");
		if (n!=kn.npos)
			key_name.assign(kn, n+1);
		else
			key_name = fname;
		if (Debug) std::cout << "C: " << key_name << " " << file_name << std::endl;

		const Aws::String bucket_name = buckname;
		const Aws::String myregion = reg;

		nCounter++;
		std::cout << nCounter << ": Uploading " << file_name << "...";
		errf << nCounter << ": Uploading " << file_name << "...";

		Aws::Auth::AWSCredentials creds;
		creds.SetAWSAccessKeyId(acct);
		creds.SetAWSSecretKey(secret);

		Aws::Client::ClientConfiguration clientConfig;
		clientConfig.region = myregion;
		Aws::S3::S3Client s3_client(creds, clientConfig);

		Aws::S3::Model::PutObjectRequest object_request;
		object_request.WithBucket(bucket_name).WithKey(key_name);

		// Binary files must also have the std::ios_base::bin flag or'ed in
		auto input_data = Aws::MakeShared<Aws::FStream>("PutObjectInputStream",
			file_name.c_str(), std::ios_base::in | std::ios_base::binary);

		object_request.SetBody(input_data);

		if (dryrun == 0)
		{
			auto put_object_outcome = s3_client.PutObject(object_request);
			int retry = maxretry;
			while (!put_object_outcome.IsSuccess() && retry > 0)
			{
				Sleep(1000);
				put_object_outcome = s3_client.PutObject(object_request);
				retry--;
			}
			if (put_object_outcome.IsSuccess())
			{
				std::cerr << "Done! Frames " << frcount << " brightness " << maxbmax << std::endl;
				errf << "Done! Frames " << frcount << " brightness " << maxbmax << std::endl;
			}
			else
			{
				std::cerr << "Upload of " << file_name << " failed after " << maxretry << " attempts - check log!" << std::endl;

				errf << "Upload of " << file_name << " failed after " << maxretry << " attempts" << std::endl;
				errf << "PutObject error: " <<
					put_object_outcome.GetError().GetExceptionName() << " " <<
					put_object_outcome.GetError().GetMessage() << std::endl;
			}
		}
		else
		{
			std::cout << std::endl << "dry run, would have sent " << file_name << std::endl;
		}
	}
	Aws::ShutdownAPI(options);
	return 0;
}
