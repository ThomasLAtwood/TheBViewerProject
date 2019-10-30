// TomGroup.h : Header file defining the structure of the TomGroup class used
//	to group controls in a user interface window.
//
//
//	Written by Thomas L. Atwood
//	P.O. Box 1089
//	West Fork, Arkansas 72774
//	(479)445-4690
//	TomAtwood@Earthlink.net
//
//	Copyright © 2010 CDC
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//	
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//

#pragma once

// This class supports the grouping together of several TomButtons
// in order to manage their group-related behavior.
class TomGroup
{
public:
	TomGroup( unsigned long GroupType, unsigned long GroupBehavior, unsigned MemberCount, void *pFirstMember, ... );
	~TomGroup(void);

	unsigned long		m_GroupType;
	unsigned			m_MemberCount;
	void				**m_pMemberPointerArray;
	unsigned long		m_GroupBehavior;
							#define GROUP_SINGLE_SELECT					0x00000001
							#define GROUP_SELECT_FIRST_OR_MULTIPLE		0x00000002
							#define GROUP_MULTIPLE_SELECT				0x00000004
							#define GROUP_ONE_TOUCHES_ALL				0x00000008
							#define GROUP_SEQUENCING					0x00000010				


	void				RespondToSelection( void *pSelectedMember );
	BOOL				IsAnyButtonChecked();
	void				SetGroupVisibility( unsigned long Visibility );
	void				InitializeMembers();
	void				HasBeenCompleted( BOOL bHasBeenCompleted );

};
