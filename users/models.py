from enum import Enum
from pydantic import BaseModel
from pydantic_extra_types.phone_numbers import PhoneNumber

class UserLogin(BaseModel):
    phn: PhoneNumber
    password: str

class UserRole(str, Enum):
    ADMIN = 'admin'
    CLIENT = 'client'

class User(UserLogin):
    username: str
    # password: str
    full_name: str | None = None
    email: str | None = None
    role: UserRole = UserRole.CLIENT

class UserInDb(BaseModel):
    # _id: int # use phn as _id field
    id: int # use phn as _id field
    hashed_password: str
    username: str
    full_name: str | None = None
    email: str | None = None
    active: bool | None = None
    

class Token(BaseModel):
    access_token: str
    token_type: str

class TokenData(BaseModel):
    username: str 