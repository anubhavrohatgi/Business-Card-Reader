#include "BusinessCard.h"

using namespace std;


BCR_BEGIN_NAMESPACE


BusinessCard::BusinessCard()
{
}


string BusinessCard::firstName()
{
	return this->_firstName;
}


string BusinessCard::lastName()
{
	return this->_lastName;
}


string BusinessCard::title()
{
	return this->_title;
}


string BusinessCard::mobilePhone()
{
	return this->_mobilePhone;
}


string BusinessCard::email()
{
	return this->_emailAddress;
}


string BusinessCard::company()
{
	return this->_companyName;
}


std::string BusinessCard::companyDescription()
{
	return this->_companyDescription;
}


string BusinessCard::businessPhone()
{
	return this->_businessPhone;
}


string BusinessCard::emailAddress()
{
	return this->_emailAddress;
}


string BusinessCard::fax()
{
	return this->_fax;
}


string BusinessCard::website()
{
	return this->_webUrl;
}


cv::Mat BusinessCard::image()
{
	return this->_img;
}


void BusinessCard::setFirstName(string firstName)
{
	this->_firstName = firstName;
}


void BusinessCard::setLastName(string lastName)
{
	this->_lastName = lastName;
}


void BusinessCard::setFirstAndLastName(string firstNlast)
{
	auto delimiters = explode(firstNlast, ' ');

	for (size_t i = 0; i < delimiters.size(); i++)
		delimiters[i][0] = toupper(delimiters[i][0]);

	if (delimiters.size() > 0)
		this->_firstName = delimiters[0];

	for (size_t i = 1; i < delimiters.size(); i++)
		this->_lastName += delimiters[i] + " ";

	trim(this->_lastName);
}


void BusinessCard::setTitle(std::string title)
{
	this->_title = title;
}


void BusinessCard::setBusinessPhone(string bPhone)
{
	this->_businessPhone = bPhone;
}


void BusinessCard::setEmail(string mail)
{
	this->_emailAddress = mail;
}


void BusinessCard::setFax(string fax)
{
	this->_fax = fax;
}


void BusinessCard::setWebSite(string url)
{
	this->_webUrl = url;
}


void BusinessCard::setImage(cv::Mat img)
{
	this->_img = img.clone();
}


void BusinessCard::setMobilePhone(string mPhone)
{
	this->_mobilePhone = mPhone;
}


void BusinessCard::setCompany(std::string company)
{
	this->_companyName = company;
}


void BusinessCard::setCompanyDescription(std::string desc)
{
	this->_companyDescription = desc;
}


void BusinessCard::setAddress(string address)
{
	this->_address = address;
}


void BusinessCard::print()
{
	cout << "First Name: " << "\t" << this->_firstName << endl;
	cout << "Last Name: " << "\t" << this->_lastName << endl;
	cout << "Title: " << "\t\t" << this->_title << endl;
	cout << "Mobile Phone: " << "\t" << this->_mobilePhone << endl;
	cout << "Email:" << "\t\t" << this->_emailAddress << endl;
	cout << endl;
	cout << "Company: \t" << this->_companyName << endl;
	cout << "Description: " << "\t" << this->_companyDescription << endl;
	cout << "Address: " << "\t" << this->_address << endl;
	cout << "Business Phone: " << "" << this->_businessPhone << endl;
	cout << "Web Address:" << "\t" << this->_webUrl << endl;
	cout << "Fax: " << "\t\t" << this->_fax << endl;
}


void BusinessCard::show()
{
	if (_img.empty())
		return;

	cv::imshow("Business Card", resize(_img, 600, 600));
	cv::waitKey(0);
}

BCR_END_NAMESPACE