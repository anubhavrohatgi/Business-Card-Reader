#include "BCRTextClassifier.h"

using namespace std;


BCR_BEGIN_NAMESPACE

// Sort by font size, if they are the same
// Sort by Y, top first, if it's the same
// Sort by X
const std::function<int(shared_ptr<OCRText>, shared_ptr<OCRText>)>
BCRTextClassifier::_sortFunc = [](PtrOCRText &a, PtrOCRText &b) {
	int fontSize = a->fontSize - b->fontSize;

	if (fontSize != 0)
		return fontSize > 0;

	int top = a->top() - b->top();

	if (top != 0)
		return top < 0;

	return a->left() < b->left();
};


BCRTextClassifier::BCRTextClassifier()
{
}


BCRTextClassifier::~BCRTextClassifier()
{
}


void BCRTextClassifier::process()
{
	preprocessText();
	findEmail();
	findWebSite();
	
	groupText();
	findNumberAreas();

	findPhoneNumbers();
	findAddress();
	findFirstLastName();
	findCompany();

	
#if BCR_DEBUG & BCR_DEBUG_TXT_GROUPS
	cout << string('/', 30) << endl;
	for (auto &i : _textGroups)
		i.debugPrint();
#endif

	clear();
}


BusinessCard &BCRTextClassifier::bussinesCard()
{
	return _bc;
}


void BCRTextClassifier::setText(vector<shared_ptr<OCRText>> strings)
{
	_text = strings;
}


// Filter strings from OCR that are highly likely "junk"/mistakes
bool BCRTextClassifier::isJunk(std::string& s)
{
	if (s.length() < 3)
		return true;

	int alnum = std::count_if(s.begin(), s.end(), ::isalnum);

	if (alnum *1. / s.length() < 0.3)
		return true;

	int alpha = 0;
	int vowel = 0;
	int nrs = 0;
	for (size_t i = 0; i < s.length(); i++)
	{
		if (isalpha(s[i]))
		{
			alpha++;

			if (is_vowel(s[i]))
				vowel++;
		}
		else if (isdigit(s[i]))
			nrs++;
	}

	if (alpha > 6 && vowel == 0)
		return true;

	if ((alpha + nrs) < s.size() / 2)
		return true;

	for (size_t i = 2; i < s.size(); i++)
		if (s[i - 2] == s[i - 1] && s[i - 1] == s[i] && !isdigit(s[i]))
			return true;

	auto words = explode(s, ' ');

	if ((alpha + nrs) * 3/4 <= words.size())
		return true;

	return false;
}


// Lines of words might've been broken into single words => merge them
// There might be multiple lines of text in a single line => break them
void BCRTextClassifier::preprocessText()
{
	vector<shared_ptr<OCRText>> newText;
	newText.reserve(_text.size());

	// Broke
	for (auto &str : _text)
	{
		auto lines = explode(str->text(), '\n');

		for (size_t j = 0; j < lines.size(); j++)
		{
			auto height = str->size().height / lines.size();
			OCRText txt(*str.get());
			txt.setText(trim(lines[j]));

			if (isJunk(txt.text()))
				continue;

			// Add data about position and size to each line
			txt.setPos(cv::Point2i(str->pos().x + j*height, str->pos().y));
			txt.setSize(cv::Size(str->size().width, height));
			txt.fontSize = height;

			newText.push_back(make_shared<OCRText>(txt));

		#if BCR_DEBUG & BCR_DEBUG_OCR_RES
			txt.debugPrint();
		#endif
		}
	}

	// Merge
	int merged = 1;
	while (merged != 0)
	{ 
		merged = 0;

		for (size_t i = 0; i < newText.size(); i++)
		{
			auto ti = newText[i];

			for (size_t j = 0; j < newText.size(); j++)
			{
				if (i == j)
					continue;

				auto tj = newText[j];

				// i should be the left one
				if (ti->right() - 10 > tj->left())
					continue;

				// Not in the same line
				if (abs(ti->middle() - tj->middle()) > 4)
					continue;

				// To far apart
				if (abs(ti->right() - tj->left()) >= 26)
					continue;

				// Merge
				ti->setText(ti->text() + " " + tj->text());
				ti->setSize(cv::Size(tj->right() - ti->left(), ti->size().height));
				newText.erase(newText.begin() + max(i, j));
				newText.erase(newText.begin() + min(i, j));
				newText.push_back(ti);
				merged++;
				break;
			}
		}
	}

	_text = newText;
}


void BCRTextClassifier::groupText()
{
	_textGroups.reserve(_text.size());

	// Every text box is it's own group at start
	for (size_t i = 0; i < _text.size(); i++)
	{
		auto group = TextGroup();
		group.push_back(_text[i]);
		_textGroups.push_back(group);
	}

	// Now merge groups
	int merged = 1;

	while (merged != 0)
	{
		merged = 0;

		for (size_t i = 0; i < _textGroups.size(); i++)
		{
			int mergeIdx = -1;
			float mergeDist = FLT_MAX;

			for (size_t j = 0; j < _textGroups.size(); j++)
			{
				if (i == j)
					continue;

				auto dist = _textGroups[i].distance(_textGroups[j]);

				if (dist == -1)
					continue;

				if (mergeDist < dist)
					continue;

				mergeIdx = j;
				mergeDist = dist;
			}

			if (mergeIdx == -1)
				continue;

			merged++;

			_textGroups[i].merge(_textGroups[mergeIdx]);
			_textGroups.erase(_textGroups.begin() + mergeIdx);
		}
	}

	_text.clear();
	// Classifier works best if things are broken to pieces
	// Break words by ',' and recreate groups
	for (auto &g : _textGroups)
	{
		int added = 0;
		for (int i = 0; i < g.size() - added; i++)
		{
			int totalLength = g[i]->text().size();
			int leftAt = g[i]->left();

			float letterSize = g[i]->size().width / totalLength;
			auto broken = explode(g[i]->text(), ',');

			for (auto &p : broken)
			{
				OCRText newLine(*g[i]);
				int width = ceil((p.size() + 1) * letterSize);

				newLine.setText(trim(p));
				newLine.setPos(cv::Point(leftAt, g[i]->pos().y));
				newLine.setSize(cv::Size(width, g[i]->size().height));
				leftAt += width;

				auto ptr = make_shared<OCRText>(newLine);
				_text.push_back(ptr);
				g.push_back(ptr);
				added++;
			}

			g.erase(g.begin() + i--);
		}
	}


	for (auto &i : _textGroups)
		i.sort();

#if BCR_DEBUG & BCR_DEBUG_TXT_GROUPS
	for (auto &i : _textGroups)
		i.debugPrint();
#endif
}


// Find areas with concentrated areas of numbers
// and classify them to phone numbers and addresses
void BCRTextClassifier::findNumberAreas()
{
	vector<NumberArea> numberAreas;
	numberAreas.reserve(4);

	for (size_t i = 0; i < _text.size(); i++)
	{
		auto str = _text[i]->text();

		auto nrOfDigits = 0;
		auto nrOfNonDigits = 0;
		auto startIndex = 0;
		auto lastNrIdx = 0;
		auto inNumberArea = false;

		const auto maxNonDigits = 3;

		for (size_t j = 0; j < str.length(); j++)
		{
			if (isspace(str[j]) ||
				isparenthesis(str[j]) ||
				str[j] == '-');

			else if (isdigit(str[j]))
			{
				if (!inNumberArea)
				{
					inNumberArea = true;
					startIndex = j;
				}

				nrOfDigits++;
				lastNrIdx = j;
			}
			else if (inNumberArea)
			{
				nrOfNonDigits++;
			}

			// If there are to many letters or we are at end of a string
			// Add substring to list and begin search for a new one
			if (inNumberArea && (nrOfNonDigits >= maxNonDigits || j + 1 >= str.length()))
			{
				if (startIndex - 1 >= 0 && _text[i]->text()[startIndex - 1] == '+')
					startIndex--;

				numberAreas.push_back({ _text[i], i, startIndex, lastNrIdx - startIndex + 1 });
				nrOfNonDigits = 0;
				nrOfDigits = 0;
				inNumberArea = false;
			}
		}
	}


#if BCR_DEBUG & BCR_DEBUG_NR_GROUPS
	BCR_DEBUG_HEADER("NUMBER AREAS")

	for (auto &i : numberAreas)
	{
		for (int j = i.startIndex; j < i.startIndex + i.length; j++)
			cout << i.text_ptr->_text[j];
		cout << endl;
	}
#endif


	for (int i = numberAreas.size() - 1; i >= 0; i--)
	{
		auto nrOfDigits = count_digits(numberAreas[i].text_ptr->text(),
			numberAreas[i].startIndex, numberAreas[i].length);

		if (nrOfDigits >= 7)
			_numbers.push_back(numberAreas[i]);
		else
			_addresses.push_back(numberAreas[i]);

		//removeText(numberAreas[i].text_ptr);
		numberAreas.erase(numberAreas.begin() + i);
	}
}


// Deletes text from _text and also updates text group(s)
bool BCRTextClassifier::removeText(shared_ptr<OCRText> &txt)
{
	auto it = find_if(_text.begin(), _text.end(), [&](shared_ptr<OCRText> &a) {
		return a == txt;
	});

	if (it == _text.end())
		return false;

	for (int i = 0; i < _textGroups.size(); i++)
	{
		int pos = _textGroups[i].position(txt);

		if (pos != INT_MAX)
		{
			_textGroups[i].erase(_textGroups[i].begin() + pos);

			if (_textGroups[i].empty())
				_textGroups.erase(_textGroups.begin() + i--);
		}
	}

	_text.erase(it);
	return true;
}


void BCRTextClassifier::clear()
{
	_text.clear();
	_textGroups.clear();
	_numbers.clear();
	_addresses.clear();
}


// Find phone number that has probably accompanying prefix eg: "fax", "tel", etc.
// Simply use distance from word to number
int BCRTextClassifier::matchPhoneNumber(std::string& search)
{
	int from = INT_MAX;
	int index = -1;

	for (int i = 0; i < _numbers.size(); i++)
	{
		auto nr1 = _numbers[i];

		auto str = nr1.text_ptr->text();
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);

		auto fPos = str.find(search);

		if (fPos == string::npos)
			continue;

		int len = nr1.startIndex - fPos;

		if (len < 0)
			continue;

		if (len < from)
		{
			index = i;
			from = len;
		}
	}

	return index;

}


void BCRTextClassifier::findEmail()
{
	// Email regex
	// Simplified version 'mail@domain will produce a match
	// OCR doesn't guarantees perfect result.
	std::smatch m;
	const std::regex e("([\\w-]+(?:\\.[\\w-]+)*)@((?:[\\w-]+\\.)*\\w[\\w-]{0,66})");

	vector<string> emails;
	emails.reserve(1);

	for (size_t i = 0; i < _text.size(); i++)
	{
		int found = 0;
		string s = to_lower(_text[i]->text());

		while (std::regex_search(s, m, e))
		{
			emails.push_back(*m.begin());
			str_delete(s, m.position(), m.length(), ' ');
			found++;
		}

		if (found > 0)
			removeText(_text[i--]);
	}

	if (emails.size() > 0)
		_bc.setEmail(emails[0]);
}


void BCRTextClassifier::findWebSite()
{
	// TODO: Move to file
	const string startsWith[] = { "www", "http" };
	const string endsWith[] = { ".com", ".eu", ".si", ".de", ".au" };

	vector<string> webUlrs;
	for (size_t i = 0; i < _text.size(); i++)
	{
		auto txt = to_lower(_text[i]->text());
		auto words = explode(txt, ' ');

		auto found = 0;
		for (size_t j = 0; j < words.size(); j++)
		{ 
			bool ok = false;

			for (auto &w : startsWith)
				if (starts_with(words[j], w))
				{
					ok = true;
					break;
				}

			if (!ok)
				for (auto &w : startsWith)
					if (ends_with(words[j], w))
					{
						ok = true;
						break;
					}

			if (!ok)
				continue;

			found++;
			webUlrs.push_back(words[j]);
		}

		if (found > 0)
			removeText(_text[i--]);
	}

	if (webUlrs.size() > 0)
		_bc.setWebSite(webUlrs[0]);
}


void BCRTextClassifier::findAddress()
{
	// Holds possible addresses
	vector<vector<NumberArea>> addr;
	for (int i = 0; i < _addresses.size(); i++)
	{
		bool sameLine = false;

		int bestMatch = -1;
		float bestDst = FLT_MAX;

		for (int j = i + 1; j < _addresses.size(); j++)
		{
			// If data is in the same line
			if (_addresses[i].text_index == _addresses[j].text_index)
			{
				bestDst = 0;
				sameLine = true;
				bestMatch = j;
			}

			if (!sameLine)
			{
				for (auto &g : _textGroups)
				{ 
					if (!g.contains(_addresses[i].text_ptr) ||
						!g.contains(_addresses[j].text_ptr))
						continue;

					auto dst = distance(_addresses[i].text_ptr->pos(), _addresses[j].text_ptr->pos());

					if (dst < bestDst)
					{
						bestDst = dst;
						bestMatch = j;
					}
				}
			}
		}

		if (bestMatch == -1)
			continue;

		if (sameLine)
		{
			if (_addresses[i].startIndex < _addresses[bestMatch].startIndex)
				addr.push_back({ _addresses[i], _addresses[bestMatch] });
			else
				addr.push_back({ _addresses[bestMatch], _addresses[i] });
		}
		else
		{
			if (_addresses[i].text_ptr->pos().x < _addresses[bestMatch].text_ptr->pos().x)
				addr.push_back({ _addresses[i], _addresses[bestMatch] });
			else
				addr.push_back({ _addresses[bestMatch], _addresses[i] });
		}

		_addresses.erase(_addresses.begin() + max(i, bestMatch));
		_addresses.erase(_addresses.begin() + min(i--, bestMatch));
	}

	if (addr.size() >= 1)
	{
		if (addr[0].size() >= 2)
		{
			auto currAddr = addr[0];

			if (currAddr[0].text_index == currAddr[1].text_index)
				_bc.setAddress(currAddr[0].text_ptr->text());
			else
				_bc.setAddress(currAddr[0].text_ptr->text() + ", " +
							   currAddr[1].text_ptr->text());

			removeText(currAddr[0].text_ptr);
			removeText(currAddr[1].text_ptr);
		}
	}
}


void BCRTextClassifier::findPhoneNumbers()
{
	struct PhoneMeta
	{
		string meta;
		void (BusinessCard::*setter)(string);
		string(BusinessCard::*getter)();
	};

	list<PhoneMeta> searchForNumbers = {
		{ "mobile", &BusinessCard::setMobilePhone,	 &BusinessCard::mobilePhone		},
		{ "fax",	&BusinessCard::setFax,			 &BusinessCard::fax				},
		{ "faks",	&BusinessCard::setFax,			 &BusinessCard::fax				},
		{ "tel",	&BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
		{ "office", &BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
		{ "phone",	&BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
		{ "fon",	&BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
		{ "mob",	&BusinessCard::setMobilePhone,	 &BusinessCard::mobilePhone		},
		{ "cell",	&BusinessCard::setMobilePhone,	 &BusinessCard::mobilePhone		},
		{ "gsm",	&BusinessCard::setMobilePhone,	 &BusinessCard::mobilePhone		},
		{ "off",	&BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
		{ "ice",	&BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
		{ "f",		&BusinessCard::setFax,			 &BusinessCard::fax				},
		{ "m",		&BusinessCard::setMobilePhone,	 &BusinessCard::mobilePhone		},
		{ "t",		&BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
		{ "c",		&BusinessCard::setMobilePhone,	 &BusinessCard::mobilePhone		},
		{ "g",		&BusinessCard::setMobilePhone,	 &BusinessCard::mobilePhone		},
		{ "el",		&BusinessCard::setBusinessPhone, &BusinessCard::businessPhone	},
	};

 	int found = 0;

	vector<NumberArea> nrs; // For used numbers
	nrs.reserve(_numbers.size());

	// Based on certain strings try to categorize phone numbers
	for (auto &i : searchForNumbers)
	{
		int bestMatchIndex = matchPhoneNumber(i.meta);

		if (bestMatchIndex < 0)
			continue;

		found++;
		auto nr = &_numbers[bestMatchIndex];

		if (!(_bc.*i.getter)().empty())
			continue;

		(_bc.*i.setter)(nr->text_ptr->text().substr(nr->startIndex, nr->length));
		nrs.push_back(*(_numbers.begin() + bestMatchIndex));
		_numbers.erase(_numbers.begin() + bestMatchIndex);
	}

	// Assign first free category to other numbers
	for (auto &i : searchForNumbers)
	{
		if (_numbers.empty())
			break;

		auto nr = &_numbers[0];

		if (!(_bc.*i.getter)().empty())
			continue;

		(_bc.*i.setter)(nr->text_ptr->text().substr(nr->startIndex, nr->length));
		nrs.push_back(*_numbers.begin());
		_numbers.erase(_numbers.begin());
	}

	// Give everything else to addresses, it might be an address
	if (!_numbers.empty())
		_addresses.insert(_addresses.end(), _numbers.begin(), _numbers.end());

	// Delete all used number strings
	for (auto &i : nrs)
		removeText(i.text_ptr);
}


void BCRTextClassifier::findFirstLastName()
{
	string mail = _bc.email();
	PtrOCRText name;

	vector<PtrOCRText> possibleNames;

	for (auto &g : _textGroups)		
		if (isGoodName(g[0]->text()))
			possibleNames.push_back(g[0]);

	// Haven't found any candidates, use everything
	if (possibleNames.empty())
	{
		for (auto &i : _text)
			if (isGoodName(i->text()))
				possibleNames.push_back(i);
	}

	if (possibleNames.empty())
		for (auto &i : _text)
		{
			auto indices = explode(i->text(), ' ');

			if (1 < indices.size() && indices.size() <= 5)
				possibleNames.push_back(i);
		}
		

	// Remove if contains digits
	for (int i = 0; i < possibleNames.size(); i++)
		if (count_digits(possibleNames[i]->text(), 0, possibleNames[i]->text().size()) >= 2)
			possibleNames.erase(possibleNames.begin() + i--);

	// Remove duplicates
	for (int i = 0; i < possibleNames.size(); i++)
		for (int j = i + 1; j < possibleNames.size(); j++)
		{
			if (levenshtein_distance(possibleNames[i]->text(), possibleNames[j]->text()) < 3)
			{
				possibleNames.erase(possibleNames.begin() + j);
				possibleNames.erase(possibleNames.begin() + i--);
				break;
			}
		}

	sort(possibleNames.begin(), possibleNames.end(), _sortFunc);

	StringMatcher matcher(possibleNames);

	if (possibleNames.size() > 1 && !_bc.email().empty())
	{
		auto atIdx = mail.find('@');
		string mailName = mail.substr(0, atIdx);

		// Annoying email beggining that deserves it's own if statement
		if (mailName != "info")
		{
			mailName.erase(remove_if(mailName.begin(), mailName.end(), [](char &c) { return !isalpha(c); }));
			matcher.match(mailName);

			name = possibleNames[matcher.getBestMatchIndex()];
		}
	}

	if (name.get() == nullptr && !possibleNames.empty())
		name = possibleNames[0];

	// Find person title
	if (name.get() != nullptr)
	{
		_bc.setFirstAndLastName(name->text());

		auto gIt = std::find_if(_textGroups.begin(), _textGroups.end(), [=](TextGroup &a) {
			return a.contains(name);
		});

		if (gIt != _textGroups.end())
		{
			int pos = gIt->position(name);
			string str;

			// Use max 3 strings bellow name
			for (int i = pos + 1; i < pos + 3 + 1 && i < gIt->size(); i++)
			{
				if (!str.empty())
					str += ", ";

				str += (*gIt)[i]->text();
				if(removeText((*gIt)[i]))
					i--;
			}

			_bc.setTitle(str);
		}

		removeText(name);
	}
}


// To much logic is bad for company name
void BCRTextClassifier::findCompany()
{
	if (_text.empty())
		return;

	// Sort largest to smalles
	sort(_text.begin(), _text.end(), _sortFunc);
	StringMatcher matcher(_text);
	shared_ptr<OCRText> companyName;

	if (!_bc.email().empty() || !_bc.website().empty())
	{
		auto url = _bc.email().empty() ? _bc.website() : _bc.email();
		auto atIdx = url.find('@');
		string company;

		// If @ doesn't exists, atIdx will be -1, and +1 equals 0 => take entire string
		auto domains = explode(url.substr(atIdx + 1, url.length()), '.');

		// extract relevant part of the domain name
		if (domains.size() >= 2)
			company = domains[domains.size() - 2];
		else
			company = domains.front();

		while (domains.size() > 0 && domains.back().size() <= 3)
			domains.pop_back();

		if (!domains.empty())
			company = domains.back();
		// end

		matcher.match(company);

		auto names = matcher.results();
		//Move very long names to the bottom, as they are probably wrong
		sort(names.begin(), names.end(), [&](PtrOCRText &a, PtrOCRText &b)
		{
			float aVal = a->fontSize;
			float bVal = b->fontSize;

			// Heuristics formula for matching: size / length_diff
			// Where length diff equals difference between current text and
			// text from domain, and int cast to clear ambiguity for abs func
			if (a->text().size() != company.size()) 
				aVal /= abs((int) (a->text().size() - company.size()));

			if (b->text().size() != company.size()) 
				bVal /= abs((int)(b->text().size() - company.size()));

			return aVal > bVal;
		});

		 
		// If there are multiple perfect results, take the largest one, otherwise
		// take the best match
		if (names.empty() && matcher.getBestMatchCost() < company.size() * 3/4)
			companyName = _text[matcher.getBestMatchIndex()];
		else if (names.size() > 0)
			companyName = names[0];
		else
			companyName = _text[0];
	}
	else
	{
		sort(_text.begin(), _text.end(), _sortFunc);

		companyName = _text[0];
	}

	// Find company title
	if (companyName.get() != nullptr)
	{
		_bc.setCompany(companyName->text());

		auto gIt = std::find_if(_textGroups.begin(), _textGroups.end(), [=](TextGroup &a) {
			return a.contains(companyName);
		});

		if (gIt != _textGroups.end())
		{
			int pos = gIt->position(companyName);

			// If there is a text that at least 1.5 the size of found text, it's probably
			// a better one
			auto it = find_if(gIt->begin(), gIt->end(), [&](PtrOCRText &a) {
				return a->fontSize*1.5 > companyName->fontSize;
			});

			int i = pos;
			// replace index if result is found
			if (it != gIt->end())
				i = it - gIt->begin();

			_bc.setCompany(gIt->at(i)->text());
			string str;

			// Use max 3 strings bellow name
			for (i+=1; i < pos + 3 + 1 && i < gIt->size(); i++)
			{
				if (!str.empty())
					str += ", ";

				str += (*gIt)[i]->text();

				if (removeText((*gIt)[i]))
					i--;
			}

			_bc.setCompanyDescription(str);
		}

		removeText(companyName);
	}
}


bool BCRTextClassifier::isGoodName(std::string str)
{
	auto words = explode(str, ' ');

	// Count words with at at least 3 characters
	int countWords = count_if(words.begin(), words.end(), [](std::string &w) {
		return w.size() > 3;
	});

	if (!(1 < countWords && countWords < 5) || words.size() > 5)
		return false;

	// Name cannot contain
	string mustNotContain [] = { "&", "und", "and", "gmbh" };
	bool ok = true;
	for (auto &s : words)
	{
		for (auto &w : mustNotContain)
			if (to_lower(s) == w)
			{
				ok = false;
				break;
			}

		if (!ok)
			break;
	}

	if (!ok)
		return false;;

	int capital = 0;
	for (auto &s : words)
	{
		if (s.size() < 3)
			continue;

		if (isalpha(s[0]) && isupper(s[0]))
			capital++;
	}

	if (capital * 1. / words.size() <= 0.5)
		return false;

	return true;
}

BCR_END_NAMESPACE