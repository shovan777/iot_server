from fastapi import FastAPI
import motor.motor_asyncio
from pydantic import BaseModel

# configure fastapi and mongo
app = FastAPI(
    title="Data collection API",
    summary="A simple application collecting data from iot devices."
)
client = motor.motor_asyncio.AsyncIOMotorClient("mongodb://127.0.0.1:27017/fire?retryWrites=true&w=majority")
db = client.fire
data_col = db.get_collection("myCollection")

print(db.list_collection_names())

# class DeviceModel(BaseModel):
#     id: Optional[PyObjectId] = Field(alias="_id", default=None)
class DataModel(BaseModel):
    temp: int
    humidity: float

@app.get("/data/{val}")
async def get_data(val: int):
    data = {"fire": val}
    print(f"******{await db.list_collection_names()}")
    return data

@app.post("/update/{dev_id}")
async def update_data(dev_id: int, data: DataModel):
    data_dict = data.model_dump()
    print(f'id: {dev_id} with data {data}')
    data_dict.update({"id": dev_id})
    return data_dict