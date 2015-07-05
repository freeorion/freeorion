class GGColor(object):
    @property
    def a(self):
        pass

    @property
    def r(self):
        pass

    @property
    def b(self):
        pass

    @property
    def g(self):
        pass


class IntBoolMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class IntDblMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class IntIntMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class IntPairVec(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def extend(self, obj):
        """
        C++ signature:
            void extend(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int> > > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def append(self, obj):
        """
        C++ signature:
            void append(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int> > > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class IntSet(object):
    def count(self, number):
        """
        C++ signature:
            unsigned int count(std::set<int,std::less<int>,std::allocator<int> >,int)
        
        :param number:
        :type number: int
        :rtype int
        """
        return int()

    def __contains__(self, number):
        """
        C++ signature:
            platform dependant
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype iter
        """
        return iter()

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()

    def empty(self):
        """
        C++ signature:
            bool empty(std::set<int,std::less<int>,std::allocator<int> >)
        
        :rtype bool
        """
        return bool()

    def size(self):
        """
        C++ signature:
            unsigned int size(std::set<int,std::less<int>,std::allocator<int> >)
        
        :rtype int
        """
        return int()


class IntSetSet(object):
    def count(self, int_set):
        """
        C++ signature:
            unsigned int count(std::set<std::set<int,std::less<int>,std::allocator<int> >,std::less<std::set<int,std::less<int>,std::allocator<int> > >,std::allocator<std::set<int,std::less<int>,std::allocator<int> > > >,std::set<int,std::less<int>,std::allocator<int> >)
        
        :param int_set:
        :type int_set: IntSet
        :rtype int
        """
        return int()

    def __contains__(self, int_set):
        """
        C++ signature:
            platform dependant
        
        :param int_set:
        :type int_set: IntSet
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()

    def empty(self):
        """
        C++ signature:
            bool empty(std::set<std::set<int,std::less<int>,std::allocator<int> >,std::less<std::set<int,std::less<int>,std::allocator<int> > >,std::allocator<std::set<int,std::less<int>,std::allocator<int> > > >)
        
        :rtype bool
        """
        return bool()

    def size(self):
        """
        C++ signature:
            unsigned int size(std::set<std::set<int,std::less<int>,std::allocator<int> >,std::less<std::set<int,std::less<int>,std::allocator<int> > >,std::allocator<std::set<int,std::less<int>,std::allocator<int> > > >)
        
        :rtype int
        """
        return int()


class IntVec(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def extend(self, obj):
        """
        C++ signature:
            void extend(std::vector<int,std::allocator<int> > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype iter
        """
        return iter()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def append(self, obj):
        """
        C++ signature:
            void append(std::vector<int,std::allocator<int> > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class ItemSpecVec(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def extend(self, obj):
        """
        C++ signature:
            void extend(std::vector<ItemSpec,std::allocator<ItemSpec> > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype iter
        """
        return iter()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def append(self, obj):
        """
        C++ signature:
            void append(std::vector<ItemSpec,std::allocator<ItemSpec> > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class MeterTypeMeterMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class MeterTypeStringPair(object):
    @property
    def meterType(self):
        pass

    @property
    def string(self):
        pass


class PairIntInt_IntMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class ShipPartMeterMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class ShipSlotVec(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def extend(self, obj):
        """
        C++ signature:
            void extend(std::vector<ShipSlotType,std::allocator<ShipSlotType> > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def append(self, obj):
        """
        C++ signature:
            void append(std::vector<ShipSlotType,std::allocator<ShipSlotType> > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class StringSet(object):
    def count(self, string):
        """
        C++ signature:
            unsigned int count(std::set<std::string,std::less<std::string >,std::allocator<std::string > >,std::string)
        
        :param string:
        :type string: str
        :rtype int
        """
        return int()

    def __contains__(self, string):
        """
        C++ signature:
            platform dependant
        
        :param string:
        :type string: str
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype iter
        """
        return iter()

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()

    def empty(self):
        """
        C++ signature:
            bool empty(std::set<std::string,std::less<std::string >,std::allocator<std::string > >)
        
        :rtype bool
        """
        return bool()

    def size(self):
        """
        C++ signature:
            unsigned int size(std::set<std::string,std::less<std::string >,std::allocator<std::string > >)
        
        :rtype int
        """
        return int()


class StringVec(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def extend(self, obj):
        """
        C++ signature:
            void extend(std::vector<std::string,std::allocator<std::string > > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype iter
        """
        return iter()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def append(self, obj):
        """
        C++ signature:
            void append(std::vector<std::string,std::allocator<std::string > > {lvalue},boost::python::api::object)
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class VisibilityIntMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class buildingType(object):
    @property
    def costTimeLocationInvariant(self):
        return bool()

    @property
    def description(self):
        return str()

    @property
    def dump(self):
        return str()

    @property
    def name(self):
        return str()

    def canBeProduced(self, number1, number2):
        """
        C++ signature:
            bool canBeProduced(BuildingType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype bool
        """
        return bool()

    def productionTime(self, number1, number2):
        """
        C++ signature:
            int productionTime(BuildingType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype int
        """
        return int()

    def perTurnCost(self, number1, number2):
        """
        C++ signature:
            float perTurnCost(BuildingType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype float
        """
        return float()

    def captureResult(self, number1, number2, number3, boolean):
        """
        C++ signature:
            CaptureResult captureResult(BuildingType {lvalue},int,int,int,bool)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :param number3:
        :type number3: int
        :param boolean:
        :type boolean: bool
        :rtype captureResult
        """
        return captureResult()

    def productionCost(self, number1, number2):
        """
        C++ signature:
            float productionCost(BuildingType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype float
        """
        return float()

    def canBeEnqueued(self, number1, number2):
        """
        C++ signature:
            bool canBeEnqueued(BuildingType,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype bool
        """
        return bool()


class diplomaticMessage(object):
    @property
    def type(self):
        return diplomaticMessageType()

    @property
    def recipient(self):
        return int()

    @property
    def sender(self):
        return int()


class diplomaticStatusUpdate(object):
    @property
    def status(self):
        pass

    @property
    def empire2(self):
        pass

    @property
    def empire1(self):
        pass


class empire(object):
    @property
    def capitalID(self):
        return int()

    @property
    def productionPoints(self):
        return float()

    @property
    def productionQueue(self):
        return productionQueue()

    @property
    def exploredSystemIDs(self):
        return IntSet()

    @property
    def planetsWithWastedPP(self):
        return IntSetSet()

    @property
    def playerName(self):
        return str()

    @property
    def fleetSupplyableSystemIDs(self):
        return IntSet()

    @property
    def planetsWithAllocatedPP(self):
        return resPoolMap()

    @property
    def availableShipParts(self):
        return StringSet()

    @property
    def availableBuildingTypes(self):
        return StringSet()

    @property
    def systemSupplyRanges(self):
        return IntIntMap()

    @property
    def allShipDesigns(self):
        return IntSet()

    @property
    def planetsWithAvailablePP(self):
        return resPoolMap()

    @property
    def researchQueue(self):
        return researchQueue()

    @property
    def empireID(self):
        return int()

    @property
    def name(self):
        return str()

    @property
    def colour(self):
        return GGColor()

    @property
    def availableShipHulls(self):
        return StringSet()

    @property
    def availableShipDesigns(self):
        return IntSet()

    @property
    def supplyUnobstructedSystems(self):
        return IntSet()

    @property
    def availableTechs(self):
        return StringSet()

    def resourceAvailable(self, resource_type):
        """
        C++ signature:
            float resourceAvailable(Empire {lvalue},ResourceType)
        
        :param resource_type:
        :type resource_type: resourceType
        :rtype float
        """
        return float()

    def techResearched(self, string):
        """
        C++ signature:
            bool techResearched(Empire {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype bool
        """
        return bool()

    def resourceProduction(self, resource_type):
        """
        C++ signature:
            float resourceProduction(Empire {lvalue},ResourceType)
        
        :param resource_type:
        :type resource_type: resourceType
        :rtype float
        """
        return float()

    def productionCostAndTime(self, production_queue_element):
        """
        C++ signature:
            std::pair<float,int> productionCostAndTime(Empire,ProductionQueue::Element)
        
        :param production_queue_element:
        :type production_queue_element: productionQueueElement
        :rtype object
        """
        return object()

    def shipDesignAvailable(self, number):
        """
        C++ signature:
            bool shipDesignAvailable(Empire {lvalue},int)
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def numSitReps(self, number):
        """
        C++ signature:
            int numSitReps(Empire {lvalue},int)
        
        :param number:
        :type number: int
        :rtype int
        """
        return int()

    def getResourcePool(self, resource_type):
        """
        C++ signature:
            boost::shared_ptr<ResourcePool> getResourcePool(Empire {lvalue},ResourceType)
        
        :param resource_type:
        :type resource_type: resourceType
        :rtype resPool
        """
        return resPool()

    def resourceStockpile(self, resource_type):
        """
        C++ signature:
            float resourceStockpile(Empire {lvalue},ResourceType)
        
        :param resource_type:
        :type resource_type: resourceType
        :rtype float
        """
        return float()

    def hasExploredSystem(self, number):
        """
        C++ signature:
            bool hasExploredSystem(Empire {lvalue},int)
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def buildingTypeAvailable(self, string):
        """
        C++ signature:
            bool buildingTypeAvailable(Empire {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype bool
        """
        return bool()

    def obstructedStarlanes(self):
        """
        C++ signature:
            std::vector<std::pair<int,int>,std::allocator<std::pair<int,int> > > obstructedStarlanes(Empire)
        
        :rtype IntPairVec
        """
        return IntPairVec()

    def population(self):
        """
        C++ signature:
            float population(Empire {lvalue})
        
        :rtype float
        """
        return float()

    def supplyProjections(self, number, boolean):
        """
        C++ signature:
            std::map<int,int,std::less<int>,std::allocator<std::pair<int const ,int> > > supplyProjections(Empire,int,bool)
        
        :param number:
        :type number: int
        :param boolean:
        :type boolean: bool
        :rtype IntIntMap
        """
        return IntIntMap()

    def canBuild(self, build_type, string, number):
        """
        C++ signatures:
            bool canBuild(Empire {lvalue},BuildType,std::string,int)
            bool canBuild(Empire {lvalue},BuildType,int,int)
        
        :param build_type:
        :type build_type: buildType
        :param string:
        :type string: str
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def getSitRep(self, number):
        """
        C++ signature:
            SitRepEntry getSitRep(Empire,int)
        
        :param number:
        :type number: int
        :rtype sitrep
        """
        return sitrep()

    def getTechStatus(self, string):
        """
        C++ signature:
            TechStatus getTechStatus(Empire {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype techStatus
        """
        return techStatus()

    def researchProgress(self, string):
        """
        C++ signature:
            float researchProgress(Empire {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype float
        """
        return float()


class fieldType(object):
    @property
    def description(self):
        return str()

    @property
    def dump(self):
        return str()

    @property
    def name(self):
        return str()


class galaxySetupData(object):
    @property
    def specialsFrequency(self):
        return galaxySetupOption()

    @property
    def age(self):
        return galaxySetupOption()

    @property
    def starlaneFrequency(self):
        return galaxySetupOption()

    @property
    def nativeFrequency(self):
        return galaxySetupOption()

    @property
    def planetDensity(self):
        return galaxySetupOption()

    @property
    def shape(self):
        return galaxyShape()

    @property
    def seed(self):
        return str()

    @property
    def monsterFrequency(self):
        return galaxySetupOption()

    @property
    def size(self):
        return int()

    @property
    def maxAIAggression(self):
        return aggression()


class hullType(object):
    @property
    def costTimeLocationInvariant(self):
        return bool()

    @property
    def name(self):
        return str()

    @property
    def numSlots(self):
        return int()

    @property
    def stealth(self):
        return float()

    @property
    def structure(self):
        return float()

    @property
    def fuel(self):
        return float()

    @property
    def slots(self):
        return ShipSlotVec()

    @property
    def speed(self):
        return float()

    @property
    def starlaneSpeed(self):
        return float()

    def productionCost(self, number1, number2):
        """
        C++ signature:
            float productionCost(HullType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype float
        """
        return float()

    def productionTime(self, number1, number2):
        """
        C++ signature:
            int productionTime(HullType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype int
        """
        return int()

    def numSlotsOfSlotType(self, ship_slot_type):
        """
        C++ signature:
            unsigned int numSlotsOfSlotType(HullType {lvalue},ShipSlotType)
        
        :param ship_slot_type:
        :type ship_slot_type: shipSlotType
        :rtype int
        """
        return int()


class itemSpec(object):
    @property
    def type(self):
        return unlockableItemType()

    @property
    def name(self):
        return str()


class meter(object):
    @property
    def current(self):
        pass

    @property
    def initial(self):
        pass

    @property
    def dump(self):
        pass


class partType(object):
    @property
    def mountableSlotTypes(self):
        return ShipSlotVec()

    @property
    def costTimeLocationInvariant(self):
        return bool()

    @property
    def capacity(self):
        return float()

    @property
    def name(self):
        return str()

    @property
    def partClass(self):
        return shipPartClass()

    def productionTime(self, number1, number2):
        """
        C++ signature:
            int productionTime(PartType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype int
        """
        return int()

    def productionCost(self, number1, number2):
        """
        C++ signature:
            float productionCost(PartType {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype float
        """
        return float()

    def canMountInSlotType(self, ship_slot_type):
        """
        C++ signature:
            bool canMountInSlotType(PartType {lvalue},ShipSlotType)
        
        :param ship_slot_type:
        :type ship_slot_type: shipSlotType
        :rtype bool
        """
        return bool()


class popCenter(object):
    @property
    def nextTurnPopGrowth(self):
        pass

    @property
    def speciesName(self):
        pass


class productionQueue(object):
    @property
    def allocatedPP(self):
        return resPoolMap()

    @property
    def empty(self):
        return bool()

    @property
    def totalSpent(self):
        return float()

    @property
    def empireID(self):
        return int()

    @property
    def size(self):
        return int()

    def __getitem__(self, number):
        """
        C++ signature:
            platform dependant
        
        :param number:
        :type number: int
        :rtype productionQueueElement
        """
        return productionQueueElement()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def objectsWithWastedPP(self, res_pool):
        """
        C++ signature:
            std::set<std::set<int,std::less<int>,std::allocator<int> >,std::less<std::set<int,std::less<int>,std::allocator<int> > >,std::allocator<std::set<int,std::less<int>,std::allocator<int> > > > objectsWithWastedPP(ProductionQueue {lvalue},boost::shared_ptr<ResourcePool>)
        
        :param res_pool:
        :type res_pool: resPool
        :rtype IntSetSet
        """
        return IntSetSet()

    def availablePP(self, res_pool):
        """
        C++ signature:
            std::map<std::set<int,std::less<int>,std::allocator<int> >,float,std::less<std::set<int,std::less<int>,std::allocator<int> > >,std::allocator<std::pair<std::set<int,std::less<int>,std::allocator<int> > const ,float> > > availablePP(ProductionQueue {lvalue},boost::shared_ptr<ResourcePool>)
        
        :param res_pool:
        :type res_pool: resPool
        :rtype resPoolMap
        """
        return resPoolMap()

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class productionQueueElement(object):
    @property
    def buildType(self):
        return buildType()

    @property
    def name(self):
        return str()

    @property
    def blocksize(self):
        return int()

    @property
    def turnsLeft(self):
        return int()

    @property
    def allocation(self):
        return float()

    @property
    def designID(self):
        return int()

    @property
    def locationID(self):
        return int()

    @property
    def progress(self):
        return float()

    @property
    def remaining(self):
        return int()


class resPool(object):
    pass


class resPoolMap(object):
    def __delitem__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype None
        """
        return None

    def __getitem__(self, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj2:
        :type obj2: object
        :rtype object
        """
        return object()

    def __contains__(self, obj):
        """
        C++ signature:
            platform dependant
        
        :param obj:
        :type obj: object
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __setitem__(self, obj1, obj2):
        """
        C++ signature:
            platform dependant
        
        :param obj1:
        :type obj1: object
        :param obj2:
        :type obj2: object
        :rtype None
        """
        return None

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()


class researchQueue(object):
    @property
    def size(self):
        return int()

    @property
    def totalSpent(self):
        return float()

    @property
    def empireID(self):
        return int()

    @property
    def empty(self):
        return bool()

    def __getitem__(self, number):
        """
        C++ signature:
            platform dependant
        
        :param number:
        :type number: int
        :rtype researchQueueElement
        """
        return researchQueueElement()

    def __contains__(self, research_queue_element):
        """
        C++ signature:
            platform dependant
        
        :param research_queue_element:
        :type research_queue_element: researchQueueElement
        :rtype bool
        """
        return bool()

    def __iter__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype object
        """
        return object()

    def __len__(self):
        """
        C++ signature:
            platform dependant
        
        :rtype int
        """
        return int()

    def inQueue(self, string):
        """
        C++ signature:
            bool inQueue(ResearchQueue {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype bool
        """
        return bool()


class researchQueueElement(object):
    @property
    def allocation(self):
        return float()

    @property
    def tech(self):
        return str()

    @property
    def turnsLeft(self):
        return int()


class resourceCenter(object):
    @property
    def turnsSinceFocusChange(self):
        pass

    @property
    def availableFoci(self):
        pass

    @property
    def focus(self):
        pass


class shipDesign(object):
    @property
    def costTimeLocationInvariant(self):
        return bool()

    @property
    def dump(self):
        return str()

    @property
    def canColonize(self):
        return bool()

    @property
    def tradeGeneration(self):
        return float()

    @property
    def detection(self):
        return float()

    @property
    def defense(self):
        return float()

    @property
    def speed(self):
        return float()

    @property
    def id(self):
        return int()

    @property
    def isMonster(self):
        return bool()

    @property
    def stealth(self):
        return float()

    @property
    def attack(self):
        return float()

    @property
    def parts(self):
        return StringVec()

    @property
    def fuel(self):
        return float()

    @property
    def shields(self):
        return float()

    @property
    def isArmed(self):
        return bool()

    @property
    def hull(self):
        return str()

    @property
    def designedOnTurn(self):
        return int()

    @property
    def colonyCapacity(self):
        return float()

    @property
    def structure(self):
        return float()

    @property
    def researchGeneration(self):
        return float()

    @property
    def canInvade(self):
        return bool()

    @property
    def attackStats(self):
        return IntVec()

    @property
    def troopCapacity(self):
        return float()

    @property
    def industryGeneration(self):
        return float()

    def productionTime(self, number1, number2):
        """
        C++ signature:
            int productionTime(ShipDesign {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype int
        """
        return int()

    def productionCost(self, number1, number2):
        """
        C++ signature:
            float productionCost(ShipDesign {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype float
        """
        return float()

    def productionLocationForEmpire(self, number1, number2):
        """
        C++ signature:
            bool productionLocationForEmpire(ShipDesign {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype bool
        """
        return bool()

    def perTurnCost(self, number1, number2):
        """
        C++ signature:
            float perTurnCost(ShipDesign {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype float
        """
        return float()

    def description(self, boolean):
        """
        C++ signature:
            std::string description(ShipDesign {lvalue},bool)
        
        :param boolean:
        :type boolean: bool
        :rtype str
        """
        return str()

    def name(self, boolean):
        """
        C++ signature:
            std::string name(ShipDesign {lvalue},bool)
        
        :param boolean:
        :type boolean: bool
        :rtype str
        """
        return str()


class sitrep(object):
    @property
    def getTags(self):
        pass

    @property
    def typeString(self):
        pass

    @property
    def getTurn(self):
        return int()

    def getDataString(self, string):
        """
        C++ signature:
            std::string getDataString(SitRepEntry {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype str
        """
        return str()

    def getDataIDNumber(self, string):
        """
        C++ signature:
            int getDataIDNumber(SitRepEntry {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype int
        """
        return int()


class special(object):
    @property
    def name(self):
        return str()

    @property
    def dump(self):
        return str()

    @property
    def spawnrate(self):
        return float()

    @property
    def spawnlimit(self):
        return int()

    @property
    def description(self):
        return str()

    def initialCapacity(self, number):
        """
        C++ signature:
            float initialCapacity(Special {lvalue},int)
        
        :param number:
        :type number: int
        :rtype float
        """
        return float()


class species(object):
    @property
    def description(self):
        return str()

    @property
    def dump(self):
        return str()

    @property
    def tags(self):
        return StringSet()

    @property
    def canColonize(self):
        return bool()

    @property
    def foci(self):
        return StringVec()

    @property
    def canProduceShips(self):
        return bool()

    @property
    def preferredFocus(self):
        return str()

    @property
    def homeworlds(self):
        return IntSet()

    @property
    def name(self):
        return str()

    def getPlanetEnvironment(self, planet_type):
        """
        C++ signature:
            PlanetEnvironment getPlanetEnvironment(Species {lvalue},PlanetType)
        
        :param planet_type:
        :type planet_type: planetType
        :rtype planetEnvironment
        """
        return planetEnvironment()


class tech(object):
    @property
    def category(self):
        return str()

    @property
    def unlockedItems(self):
        return ItemSpecVec()

    @property
    def description(self):
        return str()

    @property
    def type(self):
        return techType()

    @property
    def unlockedTechs(self):
        return StringSet()

    @property
    def prerequisites(self):
        return StringSet()

    @property
    def shortDescription(self):
        return str()

    @property
    def name(self):
        return str()

    def researchCost(self, number):
        """
        C++ signature:
            float researchCost(Tech {lvalue},int)
        
        :param number:
        :type number: int
        :rtype float
        """
        return float()

    def recursivePrerequisites(self, number):
        """
        C++ signature:
            std::vector<std::string,std::allocator<std::string > > recursivePrerequisites(Tech,int)
        
        :param number:
        :type number: int
        :rtype StringVec
        """
        return StringVec()

    def perTurnCost(self, number):
        """
        C++ signature:
            float perTurnCost(Tech {lvalue},int)
        
        :param number:
        :type number: int
        :rtype float
        """
        return float()

    def researchTime(self, number):
        """
        C++ signature:
            int researchTime(Tech {lvalue},int)
        
        :param number:
        :type number: int
        :rtype int
        """
        return int()


class universe(object):
    @property
    def shipIDs(self):
        return IntVec()

    @property
    def fieldIDs(self):
        return IntVec()

    @property
    def fleetIDs(self):
        return IntVec()

    @property
    def planetIDs(self):
        return IntVec()

    @property
    def buildingIDs(self):
        return IntVec()

    @property
    def allObjectIDs(self):
        return IntVec()

    @property
    def systemIDs(self):
        return IntVec()

    def jumpDistance(self, number1, number2):
        """
        C++ signature:
            int jumpDistance(Universe,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype int
        """
        return int()

    def dump(self):
        """
        C++ signature:
            void dump(Universe)
        
        :rtype None
        """
        return None

    def linearDistance(self, number1, number2):
        """
        C++ signature:
            double linearDistance(Universe,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype float
        """
        return float()

    def getObject(self, number):
        """
        C++ signature:
            UniverseObject const * getObject(Universe,int)
        
        :param number:
        :type number: int
        :rtype universeObject
        """
        return universeObject()

    def getPlanet(self, number):
        """
        C++ signature:
            Planet const * getPlanet(Universe,int)
        
        :param number:
        :type number: int
        :rtype planet
        """
        return planet()

    def getShip(self, number):
        """
        C++ signature:
            Ship const * getShip(Universe,int)
        
        :param number:
        :type number: int
        :rtype ship
        """
        return ship()

    def systemsConnected(self, number1, number2, number3):
        """
        C++ signature:
            bool systemsConnected(Universe,int,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :param number3:
        :type number3: int
        :rtype bool
        """
        return bool()

    def getVisibilityTurnsMap(self, number1, number2):
        """
        C++ signature:
            std::map<Visibility,int,std::less<Visibility>,std::allocator<std::pair<Visibility const ,int> > > getVisibilityTurnsMap(Universe {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype VisibilityIntMap
        """
        return dict()

    def leastJumpsPath(self, number1, number2, number3):
        """
        C++ signature:
            std::vector<int,std::allocator<int> > leastJumpsPath(Universe,int,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :param number3:
        :type number3: int
        :rtype IntVec
        """
        return IntVec()

    def getFleet(self, number):
        """
        C++ signature:
            Fleet const * getFleet(Universe,int)
        
        :param number:
        :type number: int
        :rtype fleet
        """
        return fleet()

    def getImmediateNeighbors(self, number1, number2):
        """
        C++ signature:
            std::vector<int,std::allocator<int> > getImmediateNeighbors(Universe,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype IntVec
        """
        return IntVec()

    def updateMeterEstimates(self, item_list):
        """
        C++ signature:
            void updateMeterEstimates(Universe,boost::python::list)
        
        :param item_list:
        :type item_list: list
        :rtype None
        """
        return None

    def getField(self, number):
        """
        C++ signature:
            Field const * getField(Universe,int)
        
        :param number:
        :type number: int
        :rtype field
        """
        return field()

    def destroyedObjectIDs(self, number):
        """
        C++ signature:
            std::set<int,std::less<int>,std::allocator<int> > destroyedObjectIDs(Universe {lvalue},int)
        
        :param number:
        :type number: int
        :rtype IntSet
        """
        return IntSet()

    def getSystemNeighborsMap(self, number1, number2):
        """
        C++ signature:
            std::map<int,double,std::less<int>,std::allocator<std::pair<int const ,double> > > getSystemNeighborsMap(Universe,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype IntDblMap
        """
        return IntDblMap()

    def systemHasStarlane(self, number1, number2):
        """
        C++ signature:
            bool systemHasStarlane(Universe {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype bool
        """
        return bool()

    def shortestPath(self, number1, number2, number3):
        """
        C++ signature:
            std::vector<int,std::allocator<int> > shortestPath(Universe,int,int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :param number3:
        :type number3: int
        :rtype IntVec
        """
        return IntVec()

    def getVisibility(self, number1, number2):
        """
        C++ signature:
            Visibility getVisibility(Universe {lvalue},int,int)
        
        :param number1:
        :type number1: int
        :param number2:
        :type number2: int
        :rtype visibility
        """
        return visibility()

    def getSystem(self, number):
        """
        C++ signature:
            System const * getSystem(Universe,int)
        
        :param number:
        :type number: int
        :rtype system
        """
        return system()

    def getBuilding(self, number):
        """
        C++ signature:
            Building const * getBuilding(Universe,int)
        
        :param number:
        :type number: int
        :rtype building
        """
        return building()


class universeObject(object):
    @property
    def dump(self):
        pass

    @property
    def unowned(self):
        pass

    @property
    def meters(self):
        pass

    @property
    def owner(self):
        pass

    @property
    def id(self):
        pass

    @property
    def containerObject(self):
        pass

    @property
    def creationTurn(self):
        pass

    @property
    def containedObjects(self):
        pass

    @property
    def tags(self):
        pass

    @property
    def ageInTurns(self):
        pass

    @property
    def systemID(self):
        pass

    @property
    def name(self):
        pass

    @property
    def specials(self):
        pass

    @property
    def y(self):
        pass

    @property
    def x(self):
        pass

    def contains(self, number):
        """
        C++ signature:
            bool contains(UniverseObject {lvalue},int)
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def hasTag(self, string):
        """
        C++ signature:
            bool hasTag(UniverseObject {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype bool
        """
        return bool()

    def nextTurnCurrentMeterValue(self, meter_type):
        """
        C++ signature:
            float nextTurnCurrentMeterValue(UniverseObject {lvalue},MeterType)
        
        :param meter_type:
        :type meter_type: meterType
        :rtype float
        """
        return float()

    def initialMeterValue(self, meter_type):
        """
        C++ signature:
            float initialMeterValue(UniverseObject {lvalue},MeterType)
        
        :param meter_type:
        :type meter_type: meterType
        :rtype float
        """
        return float()

    def containedBy(self, number):
        """
        C++ signature:
            bool containedBy(UniverseObject {lvalue},int)
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def currentMeterValue(self, meter_type):
        """
        C++ signature:
            float currentMeterValue(UniverseObject {lvalue},MeterType)
        
        :param meter_type:
        :type meter_type: meterType
        :rtype float
        """
        return float()

    def specialAddedOnTurn(self, string):
        """
        C++ signature:
            int specialAddedOnTurn(UniverseObject {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype int
        """
        return int()

    def ownedBy(self, number):
        """
        C++ signature:
            bool ownedBy(UniverseObject {lvalue},int)
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def getMeter(self, meter_type):
        """
        C++ signature:
            Meter const * getMeter(UniverseObject {lvalue},MeterType)
        
        :param meter_type:
        :type meter_type: meterType
        :rtype meter
        """
        return meter()

    def hasSpecial(self, string):
        """
        C++ signature:
            bool hasSpecial(UniverseObject {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype bool
        """
        return bool()


class building(universeObject):
    @property
    def buildingTypeName(self):
        return str()

    @property
    def producedByEmpireID(self):
        return int()

    @property
    def planetID(self):
        return int()

    @property
    def orderedScrapped(self):
        return bool()


class field(universeObject):
    @property
    def fieldTypeName(self):
        pass

    def inField(self, base_object):
        """
        C++ signatures:
            bool inField(Field,UniverseObject)
            bool inField(Field {lvalue},double,double)
        
        :param base_object:
        :type base_object: universeObject
        :rtype bool
        """
        return bool()


class fleet(universeObject):
    @property
    def hasOutpostShips(self):
        return bool()

    @property
    def shipIDs(self):
        return IntSet()

    @property
    def numShips(self):
        return int()

    @property
    def speed(self):
        return float()

    @property
    def previousSystemID(self):
        return int()

    @property
    def hasColonyShips(self):
        return bool()

    @property
    def canChangeDirectionEnRoute(self):
        return bool()

    @property
    def nextSystemID(self):
        return int()

    @property
    def finalDestinationID(self):
        return int()

    @property
    def hasMonsters(self):
        return bool()

    @property
    def hasArmedShips(self):
        return bool()

    @property
    def fuel(self):
        return float()

    @property
    def aggressive(self):
        return bool()

    @property
    def hasTroopShips(self):
        return bool()

    @property
    def maxFuel(self):
        return float()

    @property
    def empty(self):
        return bool()


class planet(universeObject, popCenter, resourceCenter):
    @property
    def originalType(self):
        return planetType()

    @property
    def nextLargerPlanetSize(self):
        return planetSize()

    @property
    def distanceFromOriginalType(self):
        return int()

    @property
    def clockwiseNextPlanetType(self):
        return planetType()

    @property
    def nextSmallerPlanetSize(self):
        return planetSize()

    @property
    def buildingIDs(self):
        return IntSet()

    @property
    def OrbitalPeriod(self):
        pass

    @property
    def counterClockwiseNextPlanetType(self):
        return planetType()

    @property
    def RotationalPeriod(self):
        pass

    @property
    def type(self):
        return planetType()

    @property
    def InitialOrbitalPosition(self):
        pass

    @property
    def size(self):
        return planetSize()

    def nextBetterPlanetTypeForSpecies(self, string):
        """
        C++ signature:
            PlanetType nextBetterPlanetTypeForSpecies(Planet {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype planetType
        """
        return planetType()

    def environmentForSpecies(self, string):
        """
        C++ signature:
            PlanetEnvironment environmentForSpecies(Planet {lvalue},std::string)
        
        :param string:
        :type string: str
        :rtype planetEnvironment
        """
        return planetEnvironment()

    def OrbitalPositionOnTurn(self, number):
        """
        C++ signature:
            Radian OrbitalPositionOnTurn(Planet {lvalue},int)
        
        :param number:
        :type number: int
        :rtype object
        """
        return object()


class ship(universeObject):
    @property
    def partMeters(self):
        return ShipPartMeterMap()

    @property
    def speciesName(self):
        return str()

    @property
    def orderedScrapped(self):
        return bool()

    @property
    def isArmed(self):
        return bool()

    @property
    def troopCapacity(self):
        return float()

    @property
    def canColonize(self):
        return bool()

    @property
    def canInvade(self):
        return bool()

    @property
    def designID(self):
        return int()

    @property
    def producedByEmpireID(self):
        return int()

    @property
    def isMonster(self):
        return bool()

    @property
    def design(self):
        return shipDesign()

    @property
    def orderedInvadePlanet(self):
        return int()

    @property
    def colonyCapacity(self):
        return float()

    @property
    def fleetID(self):
        return int()

    @property
    def canBombard(self):
        return bool()

    @property
    def speed(self):
        return float()

    @property
    def orderedColonizePlanet(self):
        return int()

    def currentPartMeterValue(self, meter_type, string):
        """
        C++ signature:
            float currentPartMeterValue(Ship {lvalue},MeterType,std::string)
        
        :param meter_type:
        :type meter_type: meterType
        :param string:
        :type string: str
        :rtype float
        """
        return float()

    def initialPartMeterValue(self, meter_type, string):
        """
        C++ signature:
            float initialPartMeterValue(Ship {lvalue},MeterType,std::string)
        
        :param meter_type:
        :type meter_type: meterType
        :param string:
        :type string: str
        :rtype float
        """
        return float()


class system(universeObject):
    @property
    def numStarlanes(self):
        return int()

    @property
    def fleetIDs(self):
        return IntSet()

    @property
    def shipIDs(self):
        return IntSet()

    @property
    def starlanesWormholes(self):
        return IntBoolMap()

    @property
    def fieldIDs(self):
        return IntSet()

    @property
    def numWormholes(self):
        return int()

    @property
    def buildingIDs(self):
        return IntSet()

    @property
    def starType(self):
        return starType()

    @property
    def lastTurnBattleHere(self):
        return int()

    @property
    def planetIDs(self):
        return IntSet()

    def HasStarlaneToSystemID(self, number):
        """
        C++ signature:
            bool HasStarlaneToSystemID(System {lvalue},int)
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()

    def HasWormholeToSystemID(self, number):
        """
        Currently unused.
        
        C++ signature:
            bool HasWormholeToSystemID(System {lvalue},int)
        
        :param number:
        :type number: int
        :rtype bool
        """
        return bool()


class Enum(int):
    """Enum stub for docs, not really present in fo"""
    pass


class aggression(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    invalid = None  # aggression(-1, "invalid")
    beginner = None  # aggression(0, "beginner")
    turtle = None  # aggression(1, "turtle")
    cautious = None  # aggression(2, "cautious")
    typical = None  # aggression(3, "typical")
    aggressive = None  # aggression(4, "aggressive")
    maniacal = None  # aggression(5, "maniacal")

aggression.invalid = aggression(-1, "invalid")
aggression.beginner = aggression(0, "beginner")
aggression.turtle = aggression(1, "turtle")
aggression.cautious = aggression(2, "cautious")
aggression.typical = aggression(3, "typical")
aggression.aggressive = aggression(4, "aggressive")
aggression.maniacal = aggression(5, "maniacal")


class buildType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    building = None  # buildType(1, "building")
    ship = None  # buildType(2, "ship")

buildType.building = buildType(1, "building")
buildType.ship = buildType(2, "ship")


class captureResult(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    capture = None  # captureResult(0, "capture")
    destroy = None  # captureResult(1, "destroy")
    retain = None  # captureResult(2, "retain")

captureResult.capture = captureResult(0, "capture")
captureResult.destroy = captureResult(1, "destroy")
captureResult.retain = captureResult(2, "retain")


class diplomaticMessageType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    noMessage = None  # diplomaticMessageType(-1, "noMessage")
    warDeclaration = None  # diplomaticMessageType(0, "warDeclaration")
    peaceProposal = None  # diplomaticMessageType(1, "peaceProposal")
    acceptProposal = None  # diplomaticMessageType(2, "acceptProposal")
    cancelProposal = None  # diplomaticMessageType(3, "cancelProposal")

diplomaticMessageType.noMessage = diplomaticMessageType(-1, "noMessage")
diplomaticMessageType.warDeclaration = diplomaticMessageType(0, "warDeclaration")
diplomaticMessageType.peaceProposal = diplomaticMessageType(1, "peaceProposal")
diplomaticMessageType.acceptProposal = diplomaticMessageType(2, "acceptProposal")
diplomaticMessageType.cancelProposal = diplomaticMessageType(3, "cancelProposal")


class diplomaticStatus(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    war = None  # diplomaticStatus(0, "war")
    peace = None  # diplomaticStatus(1, "peace")

diplomaticStatus.war = diplomaticStatus(0, "war")
diplomaticStatus.peace = diplomaticStatus(1, "peace")


class galaxySetupOption(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    invalid = None  # galaxySetupOption(-1, "invalid")
    none = None  # galaxySetupOption(0, "none")
    low = None  # galaxySetupOption(1, "low")
    medium = None  # galaxySetupOption(2, "medium")
    high = None  # galaxySetupOption(3, "high")

galaxySetupOption.invalid = galaxySetupOption(-1, "invalid")
galaxySetupOption.none = galaxySetupOption(0, "none")
galaxySetupOption.low = galaxySetupOption(1, "low")
galaxySetupOption.medium = galaxySetupOption(2, "medium")
galaxySetupOption.high = galaxySetupOption(3, "high")


class galaxyShape(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    invalid = None  # galaxyShape(-1, "invalid")
    spiral2 = None  # galaxyShape(0, "spiral2")
    spiral3 = None  # galaxyShape(1, "spiral3")
    spiral4 = None  # galaxyShape(2, "spiral4")
    cluster = None  # galaxyShape(3, "cluster")
    elliptical = None  # galaxyShape(4, "elliptical")
    irregular1 = None  # galaxyShape(5, "irregular1")
    irregular2 = None  # galaxyShape(6, "irregular2")
    ring = None  # galaxyShape(7, "ring")
    random = None  # galaxyShape(8, "random")

galaxyShape.invalid = galaxyShape(-1, "invalid")
galaxyShape.spiral2 = galaxyShape(0, "spiral2")
galaxyShape.spiral3 = galaxyShape(1, "spiral3")
galaxyShape.spiral4 = galaxyShape(2, "spiral4")
galaxyShape.cluster = galaxyShape(3, "cluster")
galaxyShape.elliptical = galaxyShape(4, "elliptical")
galaxyShape.irregular1 = galaxyShape(5, "irregular1")
galaxyShape.irregular2 = galaxyShape(6, "irregular2")
galaxyShape.ring = galaxyShape(7, "ring")
galaxyShape.random = galaxyShape(8, "random")


class meterType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    targetPopulation = None  # meterType(0, "targetPopulation")
    targetIndustry = None  # meterType(1, "targetIndustry")
    targetResearch = None  # meterType(2, "targetResearch")
    targetTrade = None  # meterType(3, "targetTrade")
    targetConstruction = None  # meterType(4, "targetConstruction")
    maxFuel = None  # meterType(6, "maxFuel")
    maxShield = None  # meterType(7, "maxShield")
    maxStructure = None  # meterType(8, "maxStructure")
    maxDefense = None  # meterType(9, "maxDefense")
    maxTroops = None  # meterType(10, "maxTroops")
    maxSupply = None  # meterType(11, "maxSupply")
    population = None  # meterType(12, "population")
    industry = None  # meterType(13, "industry")
    research = None  # meterType(14, "research")
    trade = None  # meterType(15, "trade")
    construction = None  # meterType(16, "construction")
    fuel = None  # meterType(18, "fuel")
    shield = None  # meterType(19, "shield")
    structure = None  # meterType(20, "structure")
    defense = None  # meterType(21, "defense")
    troops = None  # meterType(22, "troops")
    supply = None  # meterType(23, "supply")
    rebels = None  # meterType(24, "rebels")
    size = None  # meterType(25, "size")
    stealth = None  # meterType(26, "stealth")
    detection = None  # meterType(27, "detection")
    starlaneSpeed = None  # meterType(28, "starlaneSpeed")
    damage = None  # meterType(29, "damage")
    capacity = None  # meterType(30, "capacity")

meterType.targetPopulation = meterType(0, "targetPopulation")
meterType.targetIndustry = meterType(1, "targetIndustry")
meterType.targetResearch = meterType(2, "targetResearch")
meterType.targetTrade = meterType(3, "targetTrade")
meterType.targetConstruction = meterType(4, "targetConstruction")
meterType.maxFuel = meterType(6, "maxFuel")
meterType.maxShield = meterType(7, "maxShield")
meterType.maxStructure = meterType(8, "maxStructure")
meterType.maxDefense = meterType(9, "maxDefense")
meterType.maxTroops = meterType(10, "maxTroops")
meterType.maxSupply = meterType(11, "maxSupply")
meterType.population = meterType(12, "population")
meterType.industry = meterType(13, "industry")
meterType.research = meterType(14, "research")
meterType.trade = meterType(15, "trade")
meterType.construction = meterType(16, "construction")
meterType.fuel = meterType(18, "fuel")
meterType.shield = meterType(19, "shield")
meterType.structure = meterType(20, "structure")
meterType.defense = meterType(21, "defense")
meterType.troops = meterType(22, "troops")
meterType.supply = meterType(23, "supply")
meterType.rebels = meterType(24, "rebels")
meterType.size = meterType(25, "size")
meterType.stealth = meterType(26, "stealth")
meterType.detection = meterType(27, "detection")
meterType.starlaneSpeed = meterType(28, "starlaneSpeed")
meterType.damage = meterType(29, "damage")
meterType.capacity = meterType(30, "capacity")


class planetEnvironment(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    uninhabitable = None  # planetEnvironment(0, "uninhabitable")
    hostile = None  # planetEnvironment(1, "hostile")
    poor = None  # planetEnvironment(2, "poor")
    adequate = None  # planetEnvironment(3, "adequate")
    good = None  # planetEnvironment(4, "good")

planetEnvironment.uninhabitable = planetEnvironment(0, "uninhabitable")
planetEnvironment.hostile = planetEnvironment(1, "hostile")
planetEnvironment.poor = planetEnvironment(2, "poor")
planetEnvironment.adequate = planetEnvironment(3, "adequate")
planetEnvironment.good = planetEnvironment(4, "good")


class planetSize(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    unknown = None  # planetSize(-1, "unknown")
    noWorld = None  # planetSize(0, "noWorld")
    tiny = None  # planetSize(1, "tiny")
    small = None  # planetSize(2, "small")
    medium = None  # planetSize(3, "medium")
    large = None  # planetSize(4, "large")
    huge = None  # planetSize(5, "huge")
    asteroids = None  # planetSize(6, "asteroids")
    gasGiant = None  # planetSize(7, "gasGiant")

planetSize.unknown = planetSize(-1, "unknown")
planetSize.noWorld = planetSize(0, "noWorld")
planetSize.tiny = planetSize(1, "tiny")
planetSize.small = planetSize(2, "small")
planetSize.medium = planetSize(3, "medium")
planetSize.large = planetSize(4, "large")
planetSize.huge = planetSize(5, "huge")
planetSize.asteroids = planetSize(6, "asteroids")
planetSize.gasGiant = planetSize(7, "gasGiant")


class planetType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    unknown = None  # planetType(-1, "unknown")
    swamp = None  # planetType(0, "swamp")
    toxic = None  # planetType(1, "toxic")
    inferno = None  # planetType(2, "inferno")
    radiated = None  # planetType(3, "radiated")
    barren = None  # planetType(4, "barren")
    tundra = None  # planetType(5, "tundra")
    desert = None  # planetType(6, "desert")
    terran = None  # planetType(7, "terran")
    ocean = None  # planetType(8, "ocean")
    asteroids = None  # planetType(9, "asteroids")
    gasGiant = None  # planetType(10, "gasGiant")

planetType.unknown = planetType(-1, "unknown")
planetType.swamp = planetType(0, "swamp")
planetType.toxic = planetType(1, "toxic")
planetType.inferno = planetType(2, "inferno")
planetType.radiated = planetType(3, "radiated")
planetType.barren = planetType(4, "barren")
planetType.tundra = planetType(5, "tundra")
planetType.desert = planetType(6, "desert")
planetType.terran = planetType(7, "terran")
planetType.ocean = planetType(8, "ocean")
planetType.asteroids = planetType(9, "asteroids")
planetType.gasGiant = planetType(10, "gasGiant")


class resourceType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    industry = None  # resourceType(0, "industry")
    trade = None  # resourceType(1, "trade")
    research = None  # resourceType(2, "research")

resourceType.industry = resourceType(0, "industry")
resourceType.trade = resourceType(1, "trade")
resourceType.research = resourceType(2, "research")


class shipPartClass(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    shortRange = None  # shipPartClass(0, "shortRange")
    missiles = None  # shipPartClass(1, "missiles")
    fighters = None  # shipPartClass(2, "fighters")
    pointDefense = None  # shipPartClass(3, "pointDefense")
    shields = None  # shipPartClass(4, "shields")
    armour = None  # shipPartClass(5, "armour")
    troops = None  # shipPartClass(6, "troops")
    detection = None  # shipPartClass(7, "detection")
    stealth = None  # shipPartClass(8, "stealth")
    fuel = None  # shipPartClass(9, "fuel")
    colony = None  # shipPartClass(10, "colony")
    speed = None  # shipPartClass(11, "speed")
    general = None  # shipPartClass(12, "general")
    bombard = None  # shipPartClass(13, "bombard")
    industry = None  # shipPartClass(14, "industry")
    research = None  # shipPartClass(15, "research")
    trade = None  # shipPartClass(16, "trade")
    productionLocation = None  # shipPartClass(17, "productionLocation")

shipPartClass.shortRange = shipPartClass(0, "shortRange")
shipPartClass.missiles = shipPartClass(1, "missiles")
shipPartClass.fighters = shipPartClass(2, "fighters")
shipPartClass.pointDefense = shipPartClass(3, "pointDefense")
shipPartClass.shields = shipPartClass(4, "shields")
shipPartClass.armour = shipPartClass(5, "armour")
shipPartClass.troops = shipPartClass(6, "troops")
shipPartClass.detection = shipPartClass(7, "detection")
shipPartClass.stealth = shipPartClass(8, "stealth")
shipPartClass.fuel = shipPartClass(9, "fuel")
shipPartClass.colony = shipPartClass(10, "colony")
shipPartClass.speed = shipPartClass(11, "speed")
shipPartClass.general = shipPartClass(12, "general")
shipPartClass.bombard = shipPartClass(13, "bombard")
shipPartClass.industry = shipPartClass(14, "industry")
shipPartClass.research = shipPartClass(15, "research")
shipPartClass.trade = shipPartClass(16, "trade")
shipPartClass.productionLocation = shipPartClass(17, "productionLocation")


class shipSlotType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    external = None  # shipSlotType(0, "external")
    internal = None  # shipSlotType(1, "internal")
    core = None  # shipSlotType(2, "core")

shipSlotType.external = shipSlotType(0, "external")
shipSlotType.internal = shipSlotType(1, "internal")
shipSlotType.core = shipSlotType(2, "core")


class starType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    unknown = None  # starType(-1, "unknown")
    blue = None  # starType(0, "blue")
    white = None  # starType(1, "white")
    yellow = None  # starType(2, "yellow")
    orange = None  # starType(3, "orange")
    red = None  # starType(4, "red")
    neutron = None  # starType(5, "neutron")
    blackHole = None  # starType(6, "blackHole")
    noStar = None  # starType(7, "noStar")

starType.unknown = starType(-1, "unknown")
starType.blue = starType(0, "blue")
starType.white = starType(1, "white")
starType.yellow = starType(2, "yellow")
starType.orange = starType(3, "orange")
starType.red = starType(4, "red")
starType.neutron = starType(5, "neutron")
starType.blackHole = starType(6, "blackHole")
starType.noStar = starType(7, "noStar")


class techStatus(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    unresearchable = None  # techStatus(0, "unresearchable")
    researchable = None  # techStatus(1, "researchable")
    complete = None  # techStatus(2, "complete")

techStatus.unresearchable = techStatus(0, "unresearchable")
techStatus.researchable = techStatus(1, "researchable")
techStatus.complete = techStatus(2, "complete")


class techType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    theory = None  # techType(0, "theory")
    application = None  # techType(1, "application")
    refinement = None  # techType(2, "refinement")

techType.theory = techType(0, "theory")
techType.application = techType(1, "application")
techType.refinement = techType(2, "refinement")


class unlockableItemType(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    invalid = None  # unlockableItemType(-1, "invalid")
    building = None  # unlockableItemType(0, "building")
    shipPart = None  # unlockableItemType(1, "shipPart")
    shipHull = None  # unlockableItemType(2, "shipHull")
    shipDesign = None  # unlockableItemType(3, "shipDesign")
    tech = None  # unlockableItemType(4, "tech")

unlockableItemType.invalid = unlockableItemType(-1, "invalid")
unlockableItemType.building = unlockableItemType(0, "building")
unlockableItemType.shipPart = unlockableItemType(1, "shipPart")
unlockableItemType.shipHull = unlockableItemType(2, "shipHull")
unlockableItemType.shipDesign = unlockableItemType(3, "shipDesign")
unlockableItemType.tech = unlockableItemType(4, "tech")


class visibility(Enum):
    def __init__(self, numerator, name):
        self.name = name
        self.numerator = numerator

    invalid = None  # visibility(-1, "invalid")
    none = None  # visibility(0, "none")
    basic = None  # visibility(1, "basic")
    partial = None  # visibility(2, "partial")
    full = None  # visibility(3, "full")

visibility.invalid = visibility(-1, "invalid")
visibility.none = visibility(0, "none")
visibility.basic = visibility(1, "basic")
visibility.partial = visibility(2, "partial")
visibility.full = visibility(3, "full")


def allEmpireIDs():
    """
    C++ signature:
        std::vector<int,std::allocator<int> > allEmpireIDs()
    :rtype IntVec
    """
    return IntVec()


def allPlayerIDs():
    """
    C++ signature:
        std::vector<int,std::allocator<int> > allPlayerIDs()
    :rtype IntVec
    """
    return IntVec()


def currentTurn():
    """
    C++ signature:
        int currentTurn()
    :rtype int
    """
    return int()


def doneTurn():
    """
    C++ signature:
        void doneTurn()
    :rtype None
    """
    return None


def empireID():
    """
    C++ signature:
        int empireID()
    :rtype int
    """
    return int()


def empirePlayerID(number):
    """
    C++ signature:
        int empirePlayerID(int)
    
    :param number:
    :type number: int
    :rtype int
    """
    return int()


def getAIConfigStr():
    """
    C++ signature:
        std::string getAIConfigStr()
    :rtype str
    """
    return str()


def getAIDir():
    """
    C++ signature:
        std::string getAIDir()
    :rtype str
    """
    return str()


def getBuildingType(string):
    """
    C++ signature:
        BuildingType const * getBuildingType(std::string)
    
    :param string:
    :type string: str
    :rtype buildingType
    """
    return buildingType()


def getEmpire():
    """
    C++ signatures:
        Empire const * getEmpire()
        Empire const * getEmpire(int)
    :rtype empire
    """
    return empire()


def getFieldType(string):
    """
    C++ signature:
        FieldType const * getFieldType(std::string)
    
    :param string:
    :type string: str
    :rtype fieldType
    """
    return fieldType()


def getGalaxySetupData():
    """
    C++ signature:
        GalaxySetupData getGalaxySetupData()
    :rtype galaxySetupData
    """
    return galaxySetupData()


def getHullType(string):
    """
    C++ signature:
        HullType const * getHullType(std::string)
    
    :param string:
    :type string: str
    :rtype hullType
    """
    return hullType()


def getPartType(string):
    """
    C++ signature:
        PartType const * getPartType(std::string)
    
    :param string:
    :type string: str
    :rtype partType
    """
    return partType()


def getSaveStateString():
    """
    C++ signature:
        std::string getSaveStateString()
    :rtype str
    """
    return str()


def getShipDesign(number):
    """
    C++ signature:
        ShipDesign const * getShipDesign(int)
    
    :param number:
    :type number: int
    :rtype shipDesign
    """
    return shipDesign()


def getSpecial(string):
    """
    C++ signature:
        Special const * getSpecial(std::string)
    
    :param string:
    :type string: str
    :rtype special
    """
    return special()


def getSpecies(string):
    """
    C++ signature:
        Species const * getSpecies(std::string)
    
    :param string:
    :type string: str
    :rtype species
    """
    return species()


def getTech(string):
    """
    C++ signature:
        Tech const * getTech(std::string)
    
    :param string:
    :type string: str
    :rtype tech
    """
    return tech()


def getTechCategories(obj):
    """
    C++ signature:
        std::vector<std::string,std::allocator<std::string > > getTechCategories(TechManager {lvalue})
    
    :param obj:
    :type obj: object
    :rtype StringVec
    """
    return StringVec()


def getUniverse():
    """
    C++ signature:
        Universe getUniverse()
    :rtype universe
    """
    return universe()


def issueAggressionOrder(number, boolean):
    """
    C++ signature:
        int issueAggressionOrder(int,bool)
    
    :param number:
    :type number: int
    :param boolean:
    :type boolean: bool
    :rtype int
    """
    return int()


def issueBombardOrder(number1, number2):
    """
    C++ signature:
        int issueBombardOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueChangeFocusOrder(number, string):
    """
    C++ signature:
        int issueChangeFocusOrder(int,std::string)
    
    :param number:
    :type number: int
    :param string:
    :type string: str
    :rtype int
    """
    return int()


def issueChangeProductionQuantityOrder(number1, number2, number3):
    """
    C++ signature:
        int issueChangeProductionQuantityOrder(int,int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :param number3:
    :type number3: int
    :rtype int
    """
    return int()


def issueColonizeOrder(number1, number2):
    """
    C++ signature:
        int issueColonizeOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueCreateShipDesignOrder(string1, string2, string3, item_list, string4, string5, boolean):
    """
    C++ signature:
        int issueCreateShipDesignOrder(std::string,std::string,std::string,boost::python::list,std::string,std::string,bool)
    
    :param string1:
    :type string1: str
    :param string2:
    :type string2: str
    :param string3:
    :type string3: str
    :param item_list:
    :type item_list: list
    :param string4:
    :type string4: str
    :param string5:
    :type string5: str
    :param boolean:
    :type boolean: bool
    :rtype int
    """
    return int()


def issueDequeueProductionOrder(number):
    """
    C++ signature:
        int issueDequeueProductionOrder(int)
    
    :param number:
    :type number: int
    :rtype int
    """
    return int()


def issueDequeueTechOrder(string):
    """
    C++ signature:
        int issueDequeueTechOrder(std::string)
    
    :param string:
    :type string: str
    :rtype int
    """
    return int()


def issueEnqueueBuildingProductionOrder(string, number):
    """
    C++ signature:
        int issueEnqueueBuildingProductionOrder(std::string,int)
    
    :param string:
    :type string: str
    :param number:
    :type number: int
    :rtype int
    """
    return int()


def issueEnqueueShipProductionOrder(number1, number2):
    """
    C++ signature:
        int issueEnqueueShipProductionOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueEnqueueTechOrder(string, number):
    """
    C++ signature:
        int issueEnqueueTechOrder(std::string,int)
    
    :param string:
    :type string: str
    :param number:
    :type number: int
    :rtype int
    """
    return int()


def issueFleetMoveOrder(number1, number2):
    """
    C++ signature:
        int issueFleetMoveOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueFleetTransferOrder(number1, number2):
    """
    C++ signature:
        int issueFleetTransferOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueGiveObjectToEmpireOrder(number1, number2):
    """
    C++ signature:
        int issueGiveObjectToEmpireOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueInvadeOrder(number1, number2):
    """
    C++ signature:
        int issueInvadeOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueNewFleetOrder(string, number):
    """
    C++ signature:
        int issueNewFleetOrder(std::string,int)
    
    :param string:
    :type string: str
    :param number:
    :type number: int
    :rtype int
    """
    return int()


def issueRenameOrder(number, string):
    """
    C++ signature:
        int issueRenameOrder(int,std::string)
    
    :param number:
    :type number: int
    :param string:
    :type string: str
    :rtype int
    """
    return int()


def issueRequeueProductionOrder(number1, number2):
    """
    C++ signature:
        int issueRequeueProductionOrder(int,int)
    
    :param number1:
    :type number1: int
    :param number2:
    :type number2: int
    :rtype int
    """
    return int()


def issueScrapOrder(number):
    """
    C++ signature:
        int issueScrapOrder(int)
    
    :param number:
    :type number: int
    :rtype int
    """
    return int()


def playerEmpireID(number):
    """
    C++ signature:
        int playerEmpireID(int)
    
    :param number:
    :type number: int
    :rtype int
    """
    return int()


def playerID():
    """
    C++ signature:
        int playerID()
    :rtype int
    """
    return int()


def playerIsAI(number):
    """
    C++ signature:
        bool playerIsAI(int)
    
    :param number:
    :type number: int
    :rtype bool
    """
    return bool()


def playerIsHost(number):
    """
    C++ signature:
        bool playerIsHost(int)
    
    :param number:
    :type number: int
    :rtype bool
    """
    return bool()


def playerName():
    """
    C++ signatures:
        std::string playerName()
        std::string playerName(int)
    :rtype str
    """
    return str()


def sendChatMessage(number, string):
    """
    C++ signature:
        void sendChatMessage(int,std::string)
    
    :param number:
    :type number: int
    :param string:
    :type string: str
    :rtype None
    """
    return None


def sendDiplomaticMessage(diplomatic_message):
    """
    C++ signature:
        void sendDiplomaticMessage(DiplomaticMessage)
    
    :param diplomatic_message:
    :type diplomatic_message: diplomaticMessage
    :rtype None
    """
    return None


def setSaveStateString(string):
    """
    C++ signature:
        void setSaveStateString(std::string)
    
    :param string:
    :type string: str
    :rtype None
    """
    return None


def techs():
    """
    C++ signature:
        std::vector<std::string,std::allocator<std::string > > techs()
    :rtype StringVec
    """
    return StringVec()


def techsInCategory(string):
    """
    C++ signature:
        std::vector<std::string,std::allocator<std::string > > techsInCategory(std::string)
    
    :param string:
    :type string: str
    :rtype StringVec
    """
    return StringVec()


def updateMeterEstimates(boolean):
    """
    C++ signature:
        void updateMeterEstimates(bool)
    
    :param boolean:
    :type boolean: bool
    :rtype None
    """
    return None


def updateProductionQueue():
    """
    C++ signature:
        void updateProductionQueue()
    :rtype None
    """
    return None


def updateResearchQueue():
    """
    C++ signature:
        void updateResearchQueue()
    :rtype None
    """
    return None


def updateResourcePools():
    """
    C++ signature:
        void updateResourcePools()
    :rtype None
    """
    return None


def userString(string):
    """
    C++ signature:
        std::string userString(std::string)
    
    :param string:
    :type string: str
    :rtype str
    """
    return str()


def userStringExists(string):
    """
    C++ signature:
        bool userStringExists(std::string)
    
    :param string:
    :type string: str
    :rtype bool
    """
    return bool()


def userStringList(string):
    """
    C++ signature:
        boost::python::list userStringList(std::string)
    
    :param string:
    :type string: str
    :rtype list
    """
    return list()


def validShipDesign(string, string_list):
    """
    C++ signatures:
        bool validShipDesign(std::string,std::vector<std::string,std::allocator<std::string > >)
        bool validShipDesign(ShipDesign)
    
    :param string:
    :type string: str
    :param string_list:
    :type string_list: StringVec
    :rtype bool
    """
    return bool()
