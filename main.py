from datetime import datetime, timedelta, timezone
from typing import Annotated
from fastapi import Depends, FastAPI, HTTPException, status
import motor.motor_asyncio
import phonenumbers
from pydantic import BaseModel
from pymongo.errors import DuplicateKeyError
from fastapi.security import OAuth2PasswordBearer, OAuth2PasswordRequestForm
from jose import JWTError, jwt
from passlib.context import CryptContext

from users.models import TokenData, User, UserInDb, Token
from config import SECRET_KEY, ALGORITHM, ACCESS_TOKEN_EXPIRE_MINUTES

# configure fastapi and mongo
app = FastAPI(
    title="Data collection API",
    summary="A simple application collecting data from iot devices.",
)

# configure dbI
client = motor.motor_asyncio.AsyncIOMotorClient(
    "mongodb://127.0.0.1:27017/fire" + "?retryWrites=true&w=majority"
)
db = client.get_database("iotDB")

users_col = db.get_collection("usersCol")

# configure auth
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")


def get_password_hash(pwd):
    return pwd_context.hash(pwd)


def salt_password(password: str) -> str:
    return "fakehashed" + password


def verify_password(plain_pass, hashed_pass):
    return pwd_context.verify(plain_pass, hashed_pass)


async def get_user(phn: int) -> UserInDb | None:
    # print(f"Helllooododododo {phn}")
    if user_dict := await users_col.find_one({"_id": int(phn)}):
        # print('*****',user_dict)
        user_dict["id"] = user_dict["_id"]
        return UserInDb(**user_dict)
    return None


async def authenticate_user(username: str, pwd: str):
    phn = int(username)
    user: UserInDb = await get_user(phn)
    if user and verify_password(salt_password(pwd), user.hashed_password):
        return user
    return False


def create_access_token(data: dict, expires_delta: timedelta = timedelta(minutes=15)):
    to_encode = data.copy()
    expires_at = datetime.now(timezone.utc) + expires_delta
    to_encode.update({"exp": expires_at})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt

async def get_current_user(token: Annotated[str, Depends(oauth2_scheme)]) -> UserInDb:
    # user = fake_decode_token(token=token)
    credentials_exception = HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid authentication creds",
            headers={"WWW-Authenticate": "Bearer"},
        )
    try:
        payload = jwt.decode(token, SECRET_KEY, algorithms=[ALGORITHM])

        username: str = payload.get("sub")
        if username is None:
            raise credentials_exception
        token_data = TokenData(username=username)
    except JWTError as e:
        print(e)
        raise credentials_exception
    user = await get_user(phn=int(token_data.username))
    if not user:
        raise credentials_exception
    return user


async def get_current_active_user(
    current_user: Annotated[UserInDb, Depends(get_current_user)],
):
    if current_user.active:
        return current_user
    raise HTTPException(status_code=400, detail="Inactive user")


# print("*********",db.list_collection_names()


# class DeviceModel(BaseModel):
#     id: Optional[PyObjectId] = Field(alias="_id", default=None)
class DataModel(BaseModel):
    temp: int
    humidity: float

@app.get("/users/me")
async def read_users_me(
    current_user: Annotated[UserInDb, Depends(get_current_active_user)]
) -> UserInDb:
    return current_user

@app.post("/users/register")
async def register_user(user_data: User):
    # register the user store in users collection
    # create a collection for that particular user
    phn = phonenumbers.parse(user_data.phn)
    user_dict = user_data.model_dump()
    user_dict["_id"] = phn.national_number
    user_dict.pop("phn")
    user_dict["hashed_password"] = get_password_hash(
        salt_password(user_dict.pop("password"))
    )
    user_dict["active"] = True
    try:
        await users_col.insert_one(user_dict)
    except DuplicateKeyError:
        return {"msg": f"User with phone {phn} already exists."}
    user_dict["id"] = user_dict["_id"]
    user = UserInDb(**user_dict)
    # I don't want to wait here how do I do it??
    await db.create_collection(f"user_{user_dict['_id']}")
    # print(f'{phn.phone_format}: {phn.__str__()}')
    return user


@app.get("/data/{dev_id}")
async def get_data(dev_id: int):
    """Get paginated sensor data from data collection.
    Order the data from latest to oldest.

    Parameters
    ----------
    dev_id : int
        Id of the device whose data we need to get.

    Returns
    -------
    list
        array of datas
    """
    data = {"fire": val}
    print(f"******{await db.list_collection_names()}")
    return data


@app.post("/update/{dev_id}")
async def update_data(dev_id: int, data: DataModel):
    """Add the sensor data of particular device to the collection.

    Parameters
    ----------
    dev_id : int
        Id of the device.
    data : DataModel
        sensor data

    Returns
    -------
    dict
        success or failure message
    """
    data_dict = data.model_dump()
    print(f"id: {dev_id} with data {data}")
    data_dict.update({"id": dev_id})
    new_data = await data_col.insert_one(data_dict)
    print("****", new_data)
    return {"message": "success"}


@app.post("/token")
async def login_for_access_token(
    user_data: Annotated[OAuth2PasswordRequestForm, Depends()]
) -> Token:
    user: UserInDb = await authenticate_user(user_data.username, user_data.password)
    print(f'****inside authenticate/ login{user}')
    if not user:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect username or password",
            headers={"WWW-Authenticate": "Bearer"},
        )
    exp_time = timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
    access_token = create_access_token(
        data={"sub": str(user.id)}, expires_delta=exp_time
    )
    return Token(access_token=access_token, token_type="bearer")
