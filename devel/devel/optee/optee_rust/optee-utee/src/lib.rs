// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#![cfg_attr(not(feature = "std"), no_std)]
#![cfg_attr(not(feature = "std"), feature(error_in_core))]

// Requires `alloc`.
#[macro_use]
extern crate alloc;

use libc_alloc::LibcAlloc;

#[global_allocator]
static ALLOCATOR: LibcAlloc = LibcAlloc;

#[cfg(not(feature = "std"))]
use core::panic::PanicInfo;
#[cfg(not(feature = "std"))]
use optee_utee_sys as raw;

#[cfg(not(feature = "std"))]
#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    unsafe { raw::TEE_Panic(0); }
    loop {}
}

pub use self::error::{Error, ErrorKind, Result};
pub use self::object::*;
pub use self::crypto_op::*;
pub use self::time::*;
pub use self::arithmetical::*;
pub use self::extension::*;
pub use self::uuid::*;
pub use self::parameter::{ParamType, ParamTypes, Parameter, Parameters};
pub use optee_utee_macros::{
    ta_close_session, ta_create, ta_destroy, ta_invoke_command, ta_open_session,
};

pub mod trace;
#[macro_use]
mod macros;
mod error;
mod parameter;
pub mod object;
pub mod crypto_op;
pub mod time;
pub mod arithmetical;
pub mod extension;
pub mod uuid;

#[cfg(feature = "std")]
pub mod net;
