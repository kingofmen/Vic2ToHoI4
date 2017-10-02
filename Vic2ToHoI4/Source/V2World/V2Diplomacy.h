/*Copyright (c) 2017 The Paradox Game Converters Project

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



#ifndef V2DIPLOMACY_H_
#define V2DIPLOMACY_H_



#include "Date.h"
#include "Object.h"
#include <memory>
#include <vector>



class V2Agreement;



class V2Diplomacy
{
	public:
		V2Diplomacy(): agreements() {}
		V2Diplomacy(shared_ptr<Object> obj);

		const vector<const V2Agreement*>& getAgreements() const	{ return agreements; }

	private:
		V2Diplomacy(const V2Diplomacy&) = delete;
		V2Diplomacy& operator=(const V2Diplomacy&) = delete;

		bool isARelevantDiplomaticObject(shared_ptr<Object> obj) const;

		vector<const V2Agreement*>	agreements;
};



#endif // V2DIPLOMACY_H_