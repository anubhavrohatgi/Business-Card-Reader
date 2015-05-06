#ifndef BC_BUSINESS_CARD_H
#define BC_BUSINESS_CARD_H

#include <iostream>
#include <string>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "bcr_common.h"


BCR_BEGIN_NAMESPACE

class BusinessCard
{
	public:
		BusinessCard();

		std::string firstName();
		std::string lastName();
		std::string title();
		std::string mobilePhone();
		std::string email();
		
		std::string company();
		std::string companyDescription();
		std::string businessPhone();
		std::string emailAddress();
		std::string fax();
		std::string website();

		cv::Mat image();


		void setFirstName(std::string firstName);
		void setLastName(std::string lastName);
		void setFirstAndLastName(std::string firstNlast);
		void setTitle(std::string title);
		void setMobilePhone(std::string mPhone);
		
		void setCompany(std::string company);
		void setCompanyDescription(std::string desc);
		void setAddress(std::string address);
		void setBusinessPhone(std::string bPhone);
		void setEmail(std::string mail);
		void setFax(std::string fax);
		void setWebSite(std::string url);

		void setImage(cv::Mat img);
		

		void print();
		void show();
		
	private:
		std::string _firstName;
		std::string _lastName;
		std::string _title;
		std::string _mobilePhone;
		std::string _emailAddress;

		std::string _companyName;
		std::string _companyDescription;
		std::string _address;
		std::string _webUrl;
		std::string _businessPhone;
		std::string _fax;

		cv::Mat _img;
};

BCR_END_NAMESPACE

#endif // BC_BUSINESS_CARD_H