/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	Includes
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

// Libkern includes
#include <libkern/c++/OSArray.h>
#include <libkern/c++/OSNumber.h>

// IOKit includes
#include <IOKit/IOLocks.h>

// SCSI Parallel Interface Family includes
#include <IOKit/scsi/spi/IOSCSIParallelInterfaceController.h>

// SCSI Architecture Model Family includes
#include "SCSIPathManagers.h"


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	Macros
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

#define DEBUG 												1
#define DEBUG_ASSERT_COMPONENT_NAME_STRING					"SCSIPathManagers"

#if DEBUG
#define SCSI_PATH_MANAGERS_DEBUGGING_LEVEL					0
#endif


#include "IOSCSIArchitectureModelFamilyDebugging.h"


#if ( SCSI_PATH_MANAGERS_DEBUGGING_LEVEL >= 1 )
#define PANIC_NOW(x)		IOPanic x
#else
#define PANIC_NOW(x)
#endif

#if ( SCSI_PATH_MANAGERS_DEBUGGING_LEVEL >= 2 )
#define ERROR_LOG(x)		IOLog x; IOSleep(1)
#else
#define ERROR_LOG(x)
#endif

#if ( SCSI_PATH_MANAGERS_DEBUGGING_LEVEL >= 3 )
#define STATUS_LOG(x)		IOLog x; IOSleep(1)
#else
#define STATUS_LOG(x)
#endif


#define kIOPropertyPathStatisticsKey		"Path Statistics"


#if 0
#pragma mark ₯ SCSIPathSet implementation
#pragma mark -
#endif


#define super OSArray
OSDefineMetaClassAndStructors ( SCSIPathSet, OSArray );


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	withCapacity - Allocates a set with desired capacity.			[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIPathSet *
SCSIPathSet::withCapacity ( unsigned int capacity )
{
	
	SCSIPathSet *	set 	= NULL;
	bool			result	= false;
	
	STATUS_LOG ( ( "+SCSIPathSet::withCapacity\n" ) );
	
	set = OSTypeAlloc ( SCSIPathSet );
	require_nonzero ( set, ErrorExit );
	
	result = set->initWithCapacity ( capacity );
	require ( result, ReleaseSet );
	
	return set;
	
	
ReleaseSet:
	
	
	require_nonzero_quiet ( set, ErrorExit );
	set->release ( );
	set = NULL;
	
	
ErrorExit:
	
	
	return set;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	setObject - Adds object to set.									[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

bool
SCSIPathSet::setObject ( const SCSITargetDevicePath * path )
{
	
	bool	result = false;
	
	STATUS_LOG ( ( "+SCSIPathSet::setObject\n" ) );
	
	result = member ( path );
	require ( ( result == false ), ErrorExit );
	
	STATUS_LOG ( ( "calling super::setObject()\n" ) );
	
	result = super::setObject ( path );
	
	STATUS_LOG ( ( "-SCSIPathSet::setObject\n" ) );
	
	return result;
	
	
ErrorExit:
	
	
	result = false;
	
	STATUS_LOG ( ( "-SCSIPathSet::setObject\n" ) );
	
	return result;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	member - Checks if object is a member of set.					[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

bool
SCSIPathSet::member ( const SCSITargetDevicePath * path ) const
{
	
	bool					result 	= false;
	SCSITargetDevicePath *	element	= NULL;
	UInt32					index	= 0;
	
	STATUS_LOG ( ( "+SCSIPathSet::member\n" ) );
	for ( index = 0; index < count; index++ )
	{
		
		element = ( SCSITargetDevicePath * ) array[index];
		if ( element->GetDomainIdentifier ( )->isEqualTo ( path->GetDomainIdentifier ( ) ) )
		{
			
			STATUS_LOG ( ( "path is member\n" ) );
			result = true;
			break;
			
		}
		
	}
	
	STATUS_LOG ( ( "-SCSIPathSet::member\n" ) );
	
	return result;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	member - Checks if object is a member of set.					[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

bool
SCSIPathSet::member ( const IOSCSIProtocolServices * interface ) const
{
	
	bool					result 		= false;
	OSNumber *				domainID	= NULL;
	SCSITargetDevicePath *	element		= NULL;
	UInt32					index		= 0;
	
	STATUS_LOG ( ( "+SCSIPathSet::member\n" ) );
	
	domainID = SCSITargetDevicePath::GetInterfaceDomainIdentifier ( interface );
	
	for ( index = 0; index < count; index++ )
	{
		
		element = ( SCSITargetDevicePath * ) array[index];
		if ( element->GetDomainIdentifier ( )->isEqualTo ( domainID ) )
		{
			
			STATUS_LOG ( ( "path is member\n" ) );
			result = true;
			break;
			
		}
		
	}
	
	STATUS_LOG ( ( "-SCSIPathSet::member\n" ) );
	
	return result;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	removeObject - Removes object from set.							[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIPathSet::removeObject ( const IOSCSIProtocolServices * interface )
{
	
	SCSITargetDevicePath *	member		= NULL;
	OSNumber *				domainID	= NULL;
	UInt32					index		= 0;
	
	STATUS_LOG ( ( "+SCSIPathSet::removeObject\n" ) );
	
	domainID = SCSITargetDevicePath::GetInterfaceDomainIdentifier ( interface );
	
	for ( index = 0; index < count; index++ )
	{
		
		member = ( SCSITargetDevicePath * ) array[index];
		if ( member->GetDomainIdentifier ( )->isEqualTo ( domainID ) )
		{
			
			super::removeObject ( index );
			break;
			
		}
		
	}
	
	STATUS_LOG ( ( "-SCSIPathSet::removeObject\n" ) );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	getAnyObject - Gets any available object in the set.			[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSITargetDevicePath *
SCSIPathSet::getAnyObject ( void ) const
{
	return ( SCSITargetDevicePath * ) super::getObject ( 0 );
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	getObject - Gets any available object in the set.				[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSITargetDevicePath *
SCSIPathSet::getObject ( unsigned int index ) const
{
	return ( SCSITargetDevicePath * ) super::getObject ( index );
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	getObjectWithInterface - Gets object with an interface equal to
//							 the interface.							[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSITargetDevicePath *
SCSIPathSet::getObjectWithInterface ( const IOSCSIProtocolServices * interface ) const
{
	
	SCSITargetDevicePath *	element		= NULL;
	SCSITargetDevicePath *	result		= NULL;
	OSNumber *				domainID	= NULL;
	UInt32					index		= 0;
	
	STATUS_LOG ( ( "+SCSIPathSet::getObjectWithInterface\n" ) );
	
	domainID = SCSITargetDevicePath::GetInterfaceDomainIdentifier ( interface );
	
	for ( index = 0; index < count; index++ )
	{
		
		element = ( SCSITargetDevicePath * ) array[index];
		if ( element->GetDomainIdentifier ( )->isEqualTo ( domainID ) )
		{
			
			result = element;
			break;
			
		}
		
	}
	
	return result;
	
}


#if 0
#pragma mark -
#pragma mark ₯ Failover Path Manager
#pragma mark -
#endif


#undef super
#define super SCSITargetDevicePathManager
OSDefineMetaClassAndStructors ( SCSIFailoverPathManager, SCSITargetDevicePathManager );


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	free - Frees any resources allocated.							[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIFailoverPathManager::free ( void )
{
	
	STATUS_LOG ( ( "SCSIFailoverPathManager::free\n" ) );
	
	if ( fPathSet != NULL )
	{
		
		fPathSet->release ( );
		fPathSet = NULL;
		
	}
	
	if ( fInactivePathSet != NULL )
	{
		
		fInactivePathSet->release ( );
		fInactivePathSet = NULL;
		
	}
	
	if ( fLock != NULL )
	{
		
		IOLockFree ( fLock );
		fLock = NULL;
		
	}
	
	super::free ( );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	Create - Static factory method used to create an instance of
//			 SCSIFailoverPathManager.						   [PUBLIC][STATIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIFailoverPathManager *
SCSIFailoverPathManager::Create ( IOSCSITargetDevice *		target,
								  IOSCSIProtocolServices * 	initialPath )
{
	
	SCSIFailoverPathManager *	manager = NULL;
	bool						result	= false;
	
	STATUS_LOG ( ( "+SCSIFailoverPathManager::Create\n" ) );
	
	manager = OSTypeAlloc ( SCSIFailoverPathManager );
	require_nonzero ( manager, ErrorExit );
	
	result = manager->InitializePathManagerForTarget ( target, initialPath );
	require ( result, ReleasePathManager );
	
	STATUS_LOG ( ( "-SCSIFailoverPathManager::Create, manager = %p\n", manager ) );
	
	return manager;
	
	
ReleasePathManager:
	
	
	require_nonzero_quiet ( manager, ErrorExit );
	manager->release ( );
	manager = NULL;
	
	
ErrorExit:
	
	
	STATUS_LOG ( ( "-SCSIRoundRobinPathManager::Create, manager = NULL\n" ) );
	
	return manager;	
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	InitializePathManagerForTarget - Initializes the path manager.  [PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

bool
SCSIFailoverPathManager::InitializePathManagerForTarget (
							IOSCSITargetDevice * 		target,
							IOSCSIProtocolServices * 	initialPath )
{
	
	bool	result = false;
	
	STATUS_LOG ( ( "SCSIFailoverPathManager::InitializePathManagerForTarget\n" ) );
	
	result = super::InitializePathManagerForTarget ( target, initialPath );
	require ( result, ErrorExit );
	
	STATUS_LOG ( ( "called super\n" ) );
	
	fLock = IOLockAlloc ( );
	require_nonzero ( fLock, ErrorExit );
	
	STATUS_LOG ( ( "allocated lock\n" ) );
	
	fPathSet = SCSIPathSet::withCapacity ( 1 );
	require_nonzero ( fPathSet, FreeLock );
	
	fInactivePathSet = SCSIPathSet::withCapacity ( 1 );
	require_nonzero ( fInactivePathSet, ReleasePathSet );
	
	STATUS_LOG ( ( "allocated path set, adding intial path\n" ) );
	
	result = AddPath ( initialPath );
	require ( result, ReleaseInactivePathSet );
	
	STATUS_LOG ( ( "added intial path, ready to go\n" ) );
	STATUS_LOG ( ( "Called AddPath, fStatistics array has %ld members\n", fStatistics->getCount ( ) ) );
	target->setProperty ( kIOPropertyPathStatisticsKey, fStatistics );	
	
	result = true;
	
	return result;
	
	
ReleaseInactivePathSet:
	
	
	require_nonzero ( fInactivePathSet, ReleasePathSet );
	fInactivePathSet->release ( );
	fInactivePathSet = NULL;
	
	
ReleasePathSet:
	
	
	require_nonzero ( fPathSet, FreeLock );
	fPathSet->release ( );
	fPathSet = NULL;
	
	
FreeLock:
	
	
	require_nonzero ( fLock, ErrorExit );
	IOLockFree ( fLock );
	fLock = NULL;
	
	
ErrorExit:
	
	
	result = false;
	
	return result;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	AddPath - Adds a path to the path manager.						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

bool
SCSIFailoverPathManager::AddPath ( IOSCSIProtocolServices * interface )
{
	
	bool					result 	= false;
	SCSITargetDevicePath *	path	= NULL;
	OSDictionary *			dict	= NULL;
	
	STATUS_LOG ( ( "SCSIFailoverPathManager::AddPath\n" ) );
	
	require_nonzero ( interface, ErrorExit );
	
	path = SCSITargetDevicePath::Create ( this, interface );
	require_nonzero ( path, ErrorExit );
	
	interface->RegisterSCSITaskCompletionRoutine ( &SCSITargetDevicePathManager::PathTaskCallback );
	
	dict = path->GetStatistics ( );
	
	STATUS_LOG ( ( "Got path stats, count = %ld\n", dict->getCount ( ) ) );
	
	fStatistics->setObject ( dict );
	
	STATUS_LOG ( ( "fStatistics array has %ld members\n", fStatistics->getCount ( ) ) );
	
	IOLockLock ( fLock );
	result = fPathSet->setObject ( path );
	IOLockUnlock ( fLock );
	
	path->release ( );
	path = NULL;
	
	
ErrorExit:
	
	
	return result;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ActivatePath - Activates a path (if its currently inactive).	   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIFailoverPathManager::ActivatePath ( IOSCSIProtocolServices * interface )
{
	
	bool					result 	= false;
	SCSITargetDevicePath *	path	= NULL;
	
	STATUS_LOG ( ( "SCSIFailoverPathManager::ActivatePath\n" ) );
	
	require_nonzero ( interface, ErrorExit );
	
	IOLockLock ( fLock );
	
	result = fInactivePathSet->member ( interface );
	if ( result == true )
	{
		
		path = fInactivePathSet->getObjectWithInterface ( interface );
		if ( path != NULL )
		{
			
			path->retain ( );
			path->Activate ( );
			fInactivePathSet->removeObject ( interface );
			fPathSet->setObject ( path );
			path->release ( );
			path = NULL;
			
		}
		
	}
	
	else
	{
		
		result = fPathSet->member ( interface );
		if ( result == false )
		{
			
			IOLockUnlock ( fLock );
			AddPath ( interface );
			goto Exit;
			
		}
		
	}
	
	IOLockUnlock ( fLock );
	
	
ErrorExit:
Exit:
	
	
	return;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	InactivatePath - Moves path to inactive list.					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIFailoverPathManager::InactivatePath ( IOSCSIProtocolServices * interface )
{
	
	bool					result 	= false;
	SCSITargetDevicePath *	path	= NULL;
	
	STATUS_LOG ( ( "SCSIFailoverPathManager::InactivatePath\n" ) );
	
	require_nonzero ( interface, ErrorExit );
	
	IOLockLock ( fLock );
	
	result = fPathSet->member ( interface );
	if ( result == true )
	{
		
		path = fPathSet->getObjectWithInterface ( interface );
		if ( path != NULL )
		{
			
			path->retain ( );
			path->Inactivate ( );
			fPathSet->removeObject ( interface );
			fInactivePathSet->setObject ( path );
			path->release ( );
			path = NULL;
			
		}
		
	}
	
	IOLockUnlock ( fLock );
	
	
ErrorExit:
	
	
	return;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	RemovePath - Removes a path from the path manager.				   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIFailoverPathManager::RemovePath ( IOSCSIProtocolServices * path )
{
	
	STATUS_LOG ( ( "SCSIFailoverPathManager::RemovePath\n" ) );
	
	if ( path != NULL )
	{
		
		IOLockLock ( fLock );
		
		// First check if it's on the inactive list (it should since the
		// notification should come before termination).
		
		if ( fInactivePathSet->member ( path ) == true )
		{
			
			SCSITargetDevicePath *	tdp = NULL;
			
			tdp = fInactivePathSet->getObjectWithInterface ( path );
			if ( tdp != NULL )
			{
				
				OSDictionary *	dict = NULL;
				
				dict = tdp->GetStatistics ( );
				
				if ( dict != NULL )
				{
					
					UInt32	count = fStatistics->getCount ( );
					UInt32	index = 0;
					
					for ( index = 0; index < count; index++ )
					{
						
						if ( fStatistics->getObject ( index ) == dict )
						{
							fStatistics->removeObject ( index );
						}
						
					}
					
				}
				
			}
			
			STATUS_LOG ( ( "Removing path from inactive path set\n" ) );
			fInactivePathSet->removeObject ( path );
			
		}
		
		else
		{
			
			SCSITargetDevicePath *	tdp = NULL;
			
			tdp = fPathSet->getObjectWithInterface ( path );
			if ( tdp != NULL )
			{
				
				OSDictionary *	dict = NULL;
				
				dict = tdp->GetStatistics ( );
				
				if ( dict != NULL )
				{
					
					UInt32	count = fStatistics->getCount ( );
					UInt32	index = 0;
					
					for ( index = 0; index < count; index++ )
					{
						
						if ( fStatistics->getObject ( index ) == dict )
						{
							fStatistics->removeObject ( index );
						}
						
					}
					
				}
				
			}
			
			ERROR_LOG ( ( "Removing path from active path set, no notification came!!!\n" ) );
			fPathSet->removeObject ( path );
			
		}
		
		IOLockUnlock ( fLock );
		
	}
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	PathStatusChanged - Notification for when a path status changes.   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIFailoverPathManager::PathStatusChanged (
							IOSCSIProtocolServices * 	path,
							SPIPortStatus				newStatus )
{
	
	STATUS_LOG ( ( "SCSIFailoverPathManager::PathStatusChanged\n" ) );
	
	switch ( newStatus )
	{
		
		case kSPIPortStatus_Online:
		{
			
			STATUS_LOG ( ( "kSPIPortStatus_Online\n" ) );
			ActivatePath ( path );
			
		}
		break;
		
		case kSPIPortStatus_Offline:
		case kSPIPortStatus_Failure:
		{
			
			STATUS_LOG ( ( "kSPIPortStatus_Offline or kSPIPortStatus_Failure\n" ) );
			InactivatePath ( path );
			
		}
		break;
		
		default:
		{
			STATUS_LOG ( ( "Unknown port status\n" ) );
		}
		break;
		
	}
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ExecuteCommand - Called to execute a SCSITask. 					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIFailoverPathManager::ExecuteCommand ( SCSITaskIdentifier request )
{
	
	SCSITargetDevicePath *		path 		= NULL;
	UInt32						numPaths	= 0;
	
	IOLockLock ( fLock );
	
	numPaths = fPathSet->getCount ( );
	
	if ( numPaths == 0 )
	{
		
		IOLockUnlock ( fLock );
		PathTaskCallback ( request );
		return;
		
	}
	
	path = fPathSet->getAnyObject ( );
	
	IOLockUnlock ( fLock );
	
	SetPathLayerReference ( request, ( void * ) path );
	path->GetInterface ( )->ExecuteCommand ( request );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	AbortTask - Called to abort a SCSITask. 						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIFailoverPathManager::AbortTask ( SCSILogicalUnitNumber 		theLogicalUnit,
									 SCSITaggedTaskIdentifier 	theTag )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->AbortTask ( theLogicalUnit, theTag );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	AbortTaskSet - Called to abort a task set. 						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIFailoverPathManager::AbortTaskSet ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->AbortTaskSet ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ClearACA - Called to clear an ACA condition. 					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIFailoverPathManager::ClearACA ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->ClearACA ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ClearTaskSet - Called to clear a task set. 						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIFailoverPathManager::ClearTaskSet ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->ClearTaskSet ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	LogicalUnitReset - Called to reset a logical unit. 				   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIFailoverPathManager::LogicalUnitReset ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->LogicalUnitReset ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	TargetReset - Called to reset a target device. 					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIFailoverPathManager::TargetReset ( void )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->TargetReset ( );
	
}


#if 0
#pragma mark -
#pragma mark ₯ Round Robin Path Manager
#pragma mark -
#endif


#undef super
#define super SCSITargetDevicePathManager
OSDefineMetaClassAndStructors ( SCSIRoundRobinPathManager, SCSITargetDevicePathManager );


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	free - Frees any resources allocated.							[PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIRoundRobinPathManager::free ( void )
{
	
	STATUS_LOG ( ( "SCSIRoundRobinPathManager::free\n" ) );
	
	if ( fPathSet != NULL )
	{
		
		fPathSet->release ( );
		fPathSet = NULL;
		
	}
	
	if ( fInactivePathSet != NULL )
	{
		
		fInactivePathSet->release ( );
		fInactivePathSet = NULL;
		
	}
	
	if ( fLock != NULL )
	{
		
		IOLockFree ( fLock );
		fLock = NULL;
		
	}
	
	super::free ( );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	Create - Static factory method used to create an instance of
//			 SCSIRoundRobinPathManager.						   [PUBLIC][STATIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIRoundRobinPathManager *
SCSIRoundRobinPathManager::Create ( IOSCSITargetDevice *		target,
								    IOSCSIProtocolServices * 	initialPath )
{
	
	SCSIRoundRobinPathManager *	manager = NULL;
	bool						result	= false;
	
	STATUS_LOG ( ( "+SCSIRoundRobinPathManager::Create\n" ) );
	
	manager = OSTypeAlloc ( SCSIRoundRobinPathManager );
	require_nonzero ( manager, ErrorExit );
	
	result = manager->InitializePathManagerForTarget ( target, initialPath );
	require ( result, ReleasePathManager );
	
	STATUS_LOG ( ( "-SCSIRoundRobinPathManager::Create, manager = %p\n", manager ) );
	
	return manager;
	
	
ReleasePathManager:
	
	
	require_nonzero_quiet ( manager, ErrorExit );
	manager->release ( );
	manager = NULL;
	
	
ErrorExit:
	
	
	STATUS_LOG ( ( "-SCSIRoundRobinPathManager::Create, manager = NULL\n" ) );
	
	return manager;	
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	InitializePathManagerForTarget - Initializes the path manager.  [PROTECTED]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

bool
SCSIRoundRobinPathManager::InitializePathManagerForTarget (
							IOSCSITargetDevice * 		target,
							IOSCSIProtocolServices * 	initialPath )
{
	
	bool	result = false;
	
	STATUS_LOG ( ( "SCSIRoundRobinPathManager::InitializePathManagerForTarget\n" ) );
	
	result = super::InitializePathManagerForTarget ( target, initialPath );
	require ( result, ErrorExit );
	
	STATUS_LOG ( ( "called super\n" ) );
	
	fLock = IOLockAlloc ( );
	require_nonzero ( fLock, ErrorExit );
	
	STATUS_LOG ( ( "allocated lock\n" ) );
	
	fPathSet = SCSIPathSet::withCapacity ( 1 );
	require_nonzero ( fPathSet, FreeLock );
	
	fInactivePathSet = SCSIPathSet::withCapacity ( 1 );
	require_nonzero ( fInactivePathSet, ReleasePathSet );
	
	STATUS_LOG ( ( "allocated path set, adding intial path\n" ) );
	
	result = AddPath ( initialPath );
	require ( result, ReleaseInactivePathSet );
	
	STATUS_LOG ( ( "added intial path, ready to go\n" ) );
	STATUS_LOG ( ( "Called AddPath, fStatistics array has %ld members\n", fStatistics->getCount ( ) ) );
	target->setProperty ( kIOPropertyPathStatisticsKey, fStatistics );	
	
	result = true;
	
	return result;
	
	
ReleaseInactivePathSet:
	
	
	require_nonzero ( fInactivePathSet, ReleasePathSet );
	fInactivePathSet->release ( );
	fInactivePathSet = NULL;
	
	
ReleasePathSet:
	
	
	require_nonzero ( fPathSet, FreeLock );
	fPathSet->release ( );
	fPathSet = NULL;
	
	
FreeLock:
	
	
	require_nonzero ( fLock, ErrorExit );
	IOLockFree ( fLock );
	fLock = NULL;
	
	
ErrorExit:
	
	
	result = false;
	
	return result;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	AddPath - Adds a path to the path manager.						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

bool
SCSIRoundRobinPathManager::AddPath ( IOSCSIProtocolServices * interface )
{
	
	bool					result 	= false;
	SCSITargetDevicePath *	path	= NULL;
	OSDictionary *			dict	= NULL;
	
	STATUS_LOG ( ( "SCSIRoundRobinPathManager::AddPath\n" ) );
	
	require_nonzero ( interface, ErrorExit );
	
	path = SCSITargetDevicePath::Create ( this, interface );
	require_nonzero ( path, ErrorExit );
	
	STATUS_LOG ( ( "Registering callback handler\n" ) );
	
	interface->RegisterSCSITaskCompletionRoutine ( &SCSITargetDevicePathManager::PathTaskCallback );
	
	dict = path->GetStatistics ( );
	
	STATUS_LOG ( ( "Got path stats, count = %ld\n", dict->getCount ( ) ) );
	
	fStatistics->setObject ( dict );
	
	STATUS_LOG ( ( "fStatistics array has %ld members\n", fStatistics->getCount ( ) ) );
	
	IOLockLock ( fLock );
	result = fPathSet->setObject ( path );
	IOLockUnlock ( fLock );
	
	path->release ( );
	path = NULL;
	
	
ErrorExit:
	
	
	return result;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ActivatePath - Activates a path (if its currently inactive).	   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIRoundRobinPathManager::ActivatePath ( IOSCSIProtocolServices * interface )
{
	
	bool					result 	= false;
	SCSITargetDevicePath *	path	= NULL;
	
	STATUS_LOG ( ( "SCSIRoundRobinPathManager::ActivatePath\n" ) );
	
	require_nonzero ( interface, ErrorExit );
	
	IOLockLock ( fLock );
	
	result = fInactivePathSet->member ( interface );
	if ( result == true )
	{
		
		path = fInactivePathSet->getObjectWithInterface ( interface );
		if ( path != NULL )
		{
			
			path->retain ( );
			path->Activate ( );
			fInactivePathSet->removeObject ( interface );
			fPathSet->setObject ( path );
			path->release ( );
			path = NULL;
			
		}
		
	}
	
	else
	{
		
		result = fPathSet->member ( interface );
		if ( result == false )
		{
			
			IOLockUnlock ( fLock );
			AddPath ( interface );
			goto Exit;
			
		}
		
	}
	
	IOLockUnlock ( fLock );
	
	
ErrorExit:
Exit:
	
	
	return;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	InactivatePath - Moves path to inactive list.					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIRoundRobinPathManager::InactivatePath ( IOSCSIProtocolServices * interface )
{
	
	bool					result 	= false;
	SCSITargetDevicePath *	path	= NULL;
	
	STATUS_LOG ( ( "SCSIRoundRobinPathManager::InactivatePath\n" ) );
	
	require_nonzero ( interface, ErrorExit );
	
	IOLockLock ( fLock );
	
	result = fPathSet->member ( interface );
	if ( result == true )
	{
		
		path = fPathSet->getObjectWithInterface ( interface );
		if ( path != NULL )
		{
			
			path->retain ( );
			path->Inactivate ( );
			fPathSet->removeObject ( interface );
			fInactivePathSet->setObject ( path );
			path->release ( );
			path = NULL;
			
		}
		
	}
	
	IOLockUnlock ( fLock );
	
	
ErrorExit:
	
	
	return;
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	RemovePath - Removes a path from the path manager.				   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIRoundRobinPathManager::RemovePath ( IOSCSIProtocolServices * path )
{
	
	STATUS_LOG ( ( "SCSIRoundRobinPathManager::RemovePath\n" ) );
	
	if ( path != NULL )
	{
		
		IOLockLock ( fLock );
		
		// First check if it's on the inactive list (it should since the
		// notification should come before termination).
		
		if ( fInactivePathSet->member ( path ) == true )
		{
			
			SCSITargetDevicePath *	tdp = NULL;
			
			tdp = fInactivePathSet->getObjectWithInterface ( path );
			if ( tdp != NULL )
			{
				
				OSDictionary *	dict = NULL;
				
				dict = tdp->GetStatistics ( );
				
				if ( dict != NULL )
				{
					
					UInt32	count = fStatistics->getCount ( );
					UInt32	index = 0;
					
					for ( index = 0; index < count; index++ )
					{
						
						if ( fStatistics->getObject ( index ) == dict )
						{
							fStatistics->removeObject ( index );
						}
						
					}
					
				}
				
			}
			
			STATUS_LOG ( ( "Removing path from inactive path set\n" ) );
			fInactivePathSet->removeObject ( path );
			
		}
		
		else
		{
			
			SCSITargetDevicePath *	tdp = NULL;
			
			tdp = fPathSet->getObjectWithInterface ( path );
			if ( tdp != NULL )
			{
				
				OSDictionary *	dict = NULL;
				
				dict = tdp->GetStatistics ( );
				
				if ( dict != NULL )
				{
					
					UInt32	count = fStatistics->getCount ( );
					UInt32	index = 0;
					
					for ( index = 0; index < count; index++ )
					{
						
						if ( fStatistics->getObject ( index ) == dict )
						{
							fStatistics->removeObject ( index );
						}
						
					}
					
				}
				
			}
			
			ERROR_LOG ( ( "Removing path from active path set, no notification came!!!\n" ) );
			fPathSet->removeObject ( path );
			
		}
		
		IOLockUnlock ( fLock );
		
	}
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	PathStatusChanged - Notification for when a path status changes.   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIRoundRobinPathManager::PathStatusChanged (
							IOSCSIProtocolServices * 	path,
							UInt32						newStatus )
{
	
	STATUS_LOG ( ( "SCSIRoundRobinPathManager::PathStatusChanged\n" ) );
	
	switch ( newStatus )
	{
		
		case kSPIPortStatus_Online:
		{
			
			STATUS_LOG ( ( "kSPIPortStatus_Online\n" ) );
			ActivatePath ( path );
			
		}
		break;
		
		case kSPIPortStatus_Offline:
		case kSPIPortStatus_Failure:
		{
			
			STATUS_LOG ( ( "kSPIPortStatus_Offline or kSPIPortStatus_Failure\n" ) );
			InactivatePath ( path );
			
		}
		break;
		
		default:
		{
			STATUS_LOG ( ( "Unknown port status\n" ) );
		}
		break;
		
	}
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ExecuteCommand - Called to execute a SCSITask. 					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

void
SCSIRoundRobinPathManager::ExecuteCommand ( SCSITaskIdentifier request )
{
	
	SCSITargetDevicePath *		path		= NULL;
	UInt32						numPaths	= 0;
	
	IOLockLock ( fLock );
	
	numPaths = fPathSet->getCount ( );
	
	if ( numPaths == 0 )
	{
		
		IOLockUnlock ( fLock );
		PathTaskCallback ( request );
		return;
		
	}
	
	if ( numPaths == 1 )
	{
		path = fPathSet->getObject ( 0 );
	}
	
	else if ( fPathNumber < numPaths )
	{
		
		path = fPathSet->getObject ( fPathNumber );
		fPathNumber++;
		
	}
	
	else
	{
		
		fPathNumber = fPathNumber % numPaths;
		path = fPathSet->getObject ( fPathNumber );
		fPathNumber++;
		
	}
	
	IOLockUnlock ( fLock );
	
	SetPathLayerReference ( request, ( void * ) path );
	path->GetInterface ( )->ExecuteCommand ( request );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	AbortTask - Called to abort a SCSITask. 						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIRoundRobinPathManager::AbortTask ( SCSILogicalUnitNumber 		theLogicalUnit,
									   SCSITaggedTaskIdentifier 	theTag )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->AbortTask ( theLogicalUnit, theTag );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	AbortTaskSet - Called to abort a task set. 						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIRoundRobinPathManager::AbortTaskSet ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->AbortTaskSet ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ClearACA - Called to clear an ACA condition. 					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIRoundRobinPathManager::ClearACA ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->ClearACA ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	ClearTaskSet - Called to clear a task set. 						   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIRoundRobinPathManager::ClearTaskSet ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->ClearTaskSet ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	LogicalUnitReset - Called to reset a logical unit. 				   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIRoundRobinPathManager::LogicalUnitReset ( SCSILogicalUnitNumber theLogicalUnit )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->LogicalUnitReset ( theLogicalUnit );
	
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	TargetReset - Called to reset a target device. 					   [PUBLIC]
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

SCSIServiceResponse
SCSIRoundRobinPathManager::TargetReset ( void )
{
	
	SCSITargetDevicePath *	path = NULL;
	
	IOLockLock ( fLock );
	path = fPathSet->getAnyObject ( );
	IOLockUnlock ( fLock );
	
	if ( path == NULL )
		return kSCSIServiceResponse_FUNCTION_REJECTED;
	
	return path->GetInterface ( )->TargetReset ( );
	
}