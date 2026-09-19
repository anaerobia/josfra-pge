<xsl:stylesheet version = '1.0'
                xmlns:xsl='http://www.w3.org/1999/XSL/Transform'    
                xmlns:xalan="http://xml.apache.org/xslt"
                xmlns:debug="debug.uri"
                extension-element-prefixes="debug"
                exclude-result-prefixes="xalan">
    
    
    <!-- Copyright (c) 2006 Raytheon Systems Company, its vendors, and                  -->
    <!-- suppliers.  ALL RIGHTS RESERVED.                                               -->


    <!-- ****************************************************************************** -->
    <!--                                                                                -->
    <!--  This stylesheet converts ODL metadata file format to DataPool XML file format -->
    <!--                                                                                -->
    <!-- ****************************************************************************** -->
     
 
    
    <xsl:output method="xml" doctype-system="http://ecsinfo.gsfc.nasa.gov/ECSInfo/ecsmetadata/dtds/DPL/ECS/ScienceGranuleMetadata.dtd" indent="yes" xalan:indent-amount="4"/>  
    
    <!-- PARAMETER section containing information not present in the input odl file --> 
    <xsl:param name="DataCenterId"/>
    <!-- <xsl:param name="SdsrvDbID"/> --> 
    <!-- <xsl:param name="ProductionDateTime"/> --> 
    <!-- <xsl:param name="InsertTime"/> --> 
    <!-- <xsl:param name="UpdateTime"/> --> 
    <!-- <xsl:param name="SizeOfGranule"/> --> 
    
    <!-- ****************************************************************************** -->
    <!-- Process the ROOT INVENTORY METADATA from the XML image of the .met            -->
    <!-- ****************************************************************************** -->
    
    <xsl:template match="INVENTORYMETADATA">
        <GranuleMetaDataFile>
            <DTDVersion>1.0</DTDVersion>
    <!--    <DataCenterId><xsl:value-of select="$DataCenterId"/></DataCenterId> --> 
            <DataCenterId>SDPTK</DataCenterId>
            <GranuleURMetaData>
                <!-- ****************************************************************************** -->
                <!--  Constructing granuleUR based on the ShortName, VersionID and sdsrvDbID        -->
                <!-- ****************************************************************************** -->
       <!--     <xsl:variable name="shortName"> -->
       <!--        <xsl:value-of  select="./COLLECTIONDESCRIPTIONCLASS/SHORTNAME/VALUE"/> -->	
       <!--     </xsl:variable> -->
       <!--     <xsl:variable name="versionID"> -->
       <!--        <xsl:value-of  select="./COLLECTIONDESCRIPTIONCLASS/VERSIONID/VALUE"/> -->	
       <!--     </xsl:variable> -->
       <!--     <xsl:choose> -->
       <!--         <xsl:when test="string-length($versionID) = 1"> -->
       <!--            <GranuleUR><xsl:value-of select="concat('SC:',$shortName,'.00',$versionID,':',$SdsrvDbID)"/></GranuleUR> -->
       <!--        </xsl:when> -->
       <!--        <xsl:when test="string-length($versionID) = 2"> -->
       <!--            <GranuleUR><xsl:value-of select="concat('SC:',$shortName,'.0',$versionID,':',$SdsrvDbID)"/></GranuleUR> -->
       <!--        </xsl:when> -->
       <!--        <xsl:otherwise> -->
       <!--            <GranuleUR><xsl:value-of select="concat('SC:',$shortName,'.',$versionID,':',$SdsrvDbID)"/></GranuleUR> -->
       <!--        </xsl:otherwise> -->
       <!--   </xsl:choose> -->
       <!--     <DbID><xsl:value-of select="$SdsrvDbID"/></DbID> -->
       <!--   <InsertTime><xsl:value-of select="$InsertTime"/></InsertTime> -->
       <!--   <LastUpdate><xsl:value-of select="$UpdateTime"/></LastUpdate> -->
                
                <!-- ****************************************************************************** -->
                <!-- The DPL CollectionMetadata(ShortName, VersionID)                               -->
                <!-- ****************************************************************************** -->   
                <CollectionMetaData>	
                    <ShortName><xsl:value-of select="./COLLECTIONDESCRIPTIONCLASS/SHORTNAME/VALUE"/></ShortName>
                    <VersionID><xsl:value-of select="./COLLECTIONDESCRIPTIONCLASS/VERSIONID/VALUE"/></VersionID>
                </CollectionMetaData>   
                
                
                <xsl:if test="child::DATAFILES">
                    <DataFiles>
                       <xsl:for-each select="./DATAFILES/DATAFILECONTAINER">
                            <DataFileContainer>
                                <xsl:if test="./DISTRIBUTEDFILENAME/VALUE != ''">
                                   <xsl:for-each select="./DISTRIBUTEDFILENAME">
                                        <DistributedFileName><xsl:value-of select="./VALUE"/></DistributedFileName>
                                   </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./FILESIZE/VALUE != ''">
                                    <xsl:for-each select="./FILESIZE">
                                        <FileSize><xsl:value-of select="./VALUE"/></FileSize>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./INTERNALECSFILENAME/VALUE != ''">
                                    <xsl:for-each select="./INTERNALECSFILENAME">
                                        <InternalECSFileName><xsl:value-of select="./VALUE"/></InternalECSFileName>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./CHECKSUMTYPE/VALUE != ''">
                                    <xsl:for-each select="./CHECKSUMTYPE">
                                        <ChecksumType><xsl:value-of select="./VALUE"/></ChecksumType>
                                    </xsl:for-each>
                                </xsl:if>
                                 <xsl:if test="./CHECKSUM/VALUE != ''">
                                   <xsl:for-each select="./CHECKSUM">
                                        <Checksum><xsl:value-of select="./VALUE"/></Checksum>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./CHECKSUMORIGIN/VALUE != ''">
                                    <xsl:for-each select="./CHECKSUMORIGIN">
                                        <ChecksumOrigin><xsl:value-of select="./VALUE"/></ChecksumOrigin>
                                    </xsl:for-each>
                                </xsl:if>
                           </DataFileContainer>
                       </xsl:for-each>
                     </DataFiles>
                 </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL ECSDataGranule                                                         -->
                <!-- (SizeMBECSDataGranule, ReprocessingPlanned?, ReprocessingActual?,              -->
                <!-- LocalGranuleID?, DayNightFlag?, ProductionDateTime, LocalVersionID?)           -->
                <!-- ****************************************************************************** -->
        
  	
                <xsl:if test="child::ECSDATAGRANULE">
                    <ECSDataGranule>
                        <!-- Check existence of SizeMBECSDataGranule. It exists in the cross DAAC data transaction-->
		
                 <!--   <xsl:choose> -->
                            <xsl:if test="./ECSDATAGRANULE/SIZEMBECSDATAGRANULE/VALUE != ''">
                                <SizeMBECSDataGranule><xsl:value-of select="./ECSDATAGRANULE/SIZEMBECSDATAGRANULE/VALUE"/></SizeMBECSDataGranule>
                              </xsl:if>
                 <!--       </xsl:when> -->
                 <!--       <xsl:otherwise> -->
                 <!--           <SizeMBECSDataGranule><xsl:value-of select="$SizeOfGranule"/></SizeMBECSDataGranule> -->
                 <!--       </xsl:otherwise> -->
                 <!--   </xsl:choose> -->
                        <xsl:if test="./ECSDATAGRANULE/REPROCESSINGPLANNED/VALUE != ''">
                            <ReprocessingPlanned><xsl:value-of select="./ECSDATAGRANULE/REPROCESSINGPLANNED/VALUE"/></ReprocessingPlanned>
                        </xsl:if>
                        <xsl:if test="./ECSDATAGRANULE/REPROCESSINGACTUAL/VALUE != ''">
                            <ReprocessingActual><xsl:value-of select="./ECSDATAGRANULE/REPROCESSINGACTUAL/VALUE"/></ReprocessingActual>
                        </xsl:if>
                        <xsl:if test="./ECSDATAGRANULE/LOCALGRANULEID/VALUE != ''">
                            <LocalGranuleID><xsl:value-of select="./ECSDATAGRANULE/LOCALGRANULEID/VALUE"/></LocalGranuleID>
                        </xsl:if>	
                        <xsl:if test="./ECSDATAGRANULE/DAYNIGHTFLAG/VALUE != ''">  
                            <DayNightFlag><xsl:value-of select="./ECSDATAGRANULE/DAYNIGHTFLAG/VALUE"/></DayNightFlag>
                        </xsl:if>
                        <!-- handle the bugs in m2xt DTD -->
                        <xsl:choose>
                            <xsl:when test="./ECSDATAGRANULE/PRODUCTIONDATETIME/VALUE != ''"> 	
                                <ProductionDateTime>
                                    <xsl:call-template name="reformat">
                                        <xsl:with-param name="dateTime" select="./ECSDATAGRANULE/PRODUCTIONDATETIME/VALUE"/>
                                    </xsl:call-template>
                                </ProductionDateTime>
                            </xsl:when>
                            <xsl:otherwise>
                              <xsl:if test="not(child::RANGEDATETIME) and not(child::SINGLEDATETIME)">
                                <ProductionDateTime><xsl:value-of select="$ProductionDateTime"/></ProductionDateTime>
                              </xsl:if>
                            </xsl:otherwise>	
                        </xsl:choose>
                        <xsl:if test="./ECSDATAGRANULE/LOCALVERSIONID/VALUE != ''">   
                            <LocalVersionID><xsl:value-of select="substring(./ECSDATAGRANULE/LOCALVERSIONID/VALUE, 1, 60)"/></LocalVersionID>
                        </xsl:if>
                    </ECSDataGranule>
                </xsl:if>
                
                
                <!-- ****************************************************************************** -->
                <!-- The DPL PGEVersionClass (PGEVersion)                                           -->
                <!-- ****************************************************************************** -->
                <xsl:if test="./PGEVERSIONCLASS/PGEVERSION/VALUE != ''">  	
                    <PGEVersionClass>		
                        <PGEVersion><xsl:value-of select="./PGEVERSIONCLASS/PGEVERSION/VALUE"/></PGEVersion>		
                    </PGEVersionClass>
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL RangeDateTime                                                          -->
                <!-- (RangeEndingTime, RangeEndingDate, RangeBeginningTime, RangeBeginningDate)     -->
                <!-- ****************************************************************************** -->  
	
                <xsl:if test="child::RANGEDATETIME">
                    <RangeDateTime>	
                        <!-- Reformat "RangeEndingTime" and "RangeBeginningTime" with DPL acceptable time format -->
                        <RangeEndingTime>	
                            <xsl:call-template name="reformat_rangetime">
                                <xsl:with-param name="time" select="./RANGEDATETIME/RANGEENDINGTIME/VALUE"/>
                            </xsl:call-template>	        	
                        </RangeEndingTime>		
                        <RangeEndingDate>
                            <xsl:call-template name="reformat_rangedate">
                                <xsl:with-param name="date" select="./RANGEDATETIME/RANGEENDINGDATE/VALUE"/>
                            </xsl:call-template>
                        </RangeEndingDate>
                        <RangeBeginningTime>		
                            <xsl:call-template name="reformat_rangetime">
                                <xsl:with-param name="time" select="./RANGEDATETIME/RANGEBEGINNINGTIME/VALUE"/>
                            </xsl:call-template>	        	
                        </RangeBeginningTime>
                        <RangeBeginningDate>
                            <xsl:call-template name="reformat_rangedate">
                                <xsl:with-param name="date" select="./RANGEDATETIME/RANGEBEGINNINGDATE/VALUE"/>
                            </xsl:call-template>
                        </RangeBeginningDate>	
                    </RangeDateTime>
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL SingleDateTime  (TimeofDay, CalendarDate)                              -->
                <!-- ****************************************************************************** -->
	
                <xsl:if test="child::SINGLEDATETIME">
                    <SingleDateTime>
                        <xsl:if test="./SINGLEDATETIME/TIMEOFDAY/VALUE != ''">
                            <TimeofDay><xsl:value-of select="./SINGLEDATETIME/TIMEOFDAY/VALUE"/></TimeofDay>
                        </xsl:if>
                        <xsl:if test="./SINGLEDATETIME/CALENDARDATE/VALUE != ''">
                            <CalendarDate><xsl:value-of select="./SINGLEDATETIME/CALENDARDATE/VALUE"/></CalendarDate>
                        </xsl:if>		
                    </SingleDateTime>
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL SpatialDomainContainer                                                 -->
                <!-- (GranuleLocality*, VerticalSpatialDomain*, HorizontalSpatialDomainContainer)   -->                                            
                <!-- ****************************************************************************** -->
	
                <xsl:if test="child::SPATIALDOMAINCONTAINER"> 
                    
                    
                    <!-- Workaround for bug in SDSRV in which it generates an empty HorizontalSpatialDomainContainer -->

                    <!-- if no VALUES are found in HorizontalSpatialDomainContainer,  it's an empty one and -->
                    <!--     must be ignored.  -->
                    <xsl:variable name="foundHorizontalValue">          
                        <xsl:for-each select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER//VALUE">
                            <xsl:choose>              
                                <xsl:when test=". != '' ">      
                                    <xsl:text>1</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:for-each>
                    </xsl:variable>               
                    
                    <!-- if no VALUES are found in SpatialDomainContainer, it's an empty one -->
                    <!--     and must to be ignored.  -->
                    <xsl:variable name="foundValue">          
                        <xsl:for-each select="./SPATIALDOMAINCONTAINER//VALUE">
                            <xsl:choose>              
                                <xsl:when test=". != '' ">      
                                    <xsl:text>1</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:for-each>
                    </xsl:variable>               
                    
                    <xsl:if test="$foundValue != '' ">        
                        <SpatialDomainContainer>
                            <xsl:if test="./SPATIALDOMAINCONTAINER/GRANULELOCALITY/LOCALITYVALUE/VALUE != ''">
                                <GranuleLocality>		
                                    <xsl:for-each select="./SPATIALDOMAINCONTAINER/GRANULELOCALITY/LOCALITYVALUE/VALUE">
                                        <LocalityValue><xsl:value-of select = "."/></LocalityValue>                        	
                                    </xsl:for-each>		
                                </GranuleLocality>
                            </xsl:if>
                            <!-- ****************************************************************************** -->
                            <!-- The DPL SpatialDomainContainer/VerticalSpatialDomain                           -->
                            <!-- (VerticalSpatialDomainContainer)                                               -->
                            <!-- ****************************************************************************** -->	
                            <xsl:if test="./SPATIALDOMAINCONTAINER/VERTICALSPATIALDOMAIN">
                                    <xsl:if test="./SPATIALDOMAINCONTAINER/VERTICALSPATIALDOMAIN/VERTICALSPATIALDOMAINCONTAINER">
                                        <xsl:for-each select="./SPATIALDOMAINCONTAINER/VERTICALSPATIALDOMAIN/VERTICALSPATIALDOMAINCONTAINER">			
                                <VerticalSpatialDomain>
                                            <VerticalSpatialDomainContainer>
                                                <xsl:if test="./VERTICALSPATIALDOMAINTYPE/VALUE !=''">
                                                    <xsl:for-each select="./VERTICALSPATIALDOMAINTYPE">			 	
                                                        <VerticalSpatialDomainType><xsl:value-of select="./VALUE"/></VerticalSpatialDomainType>
                                                    </xsl:for-each>
                                                </xsl:if>				
                                                <xsl:if test="./VERTICALSPATIALDOMAINVALUE/VALUE !=''">
                                                    <xsl:for-each select="./VERTICALSPATIALDOMAINVALUE">					
                                                        <VerticalSpatialDomainValue><xsl:value-of select="./VALUE"/></VerticalSpatialDomainValue>
                                                    </xsl:for-each>
                                                </xsl:if>				
                                            </VerticalSpatialDomainContainer>
                                </VerticalSpatialDomain>
                                        </xsl:for-each>
                                    </xsl:if>			
                            </xsl:if>
                            
                            <!-- ****************************************************************************** -->
                            <!-- The DPL SpatialDomainContainer/HorizontalSpatialDomainContainer                -->
                            <!-- (ZoneIdentifierClass?, (Point | Circle | BoundingRectangle | GPolygon))        -->                                             
                            <!-- ****************************************************************************** -->

                            <xsl:if test="$foundHorizontalValue != '' ">        	
                                <!--xsl:if test="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER"-->
                                <HorizontalSpatialDomainContainer>
                                    <xsl:if test="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/ZONEIDENTIFIERCLASS/ZONEIDENTIFIER/VALUE != ''">
                                        <ZoneIdentifierClass>
                                            <ZoneIdentifier><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/ZONEIDENTIFIERCLASS/ZONEIDENTIFIER/VALUE"/></ZoneIdentifier>
                                        </ZoneIdentifierClass>
                                    </xsl:if>
                                    <xsl:if test="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/CIRCLE">
                                        <Circle>				
                                            <CenterLatitude><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/CIRCLE/CENTERLATITUDE/VALUE"/></CenterLatitude>
                                            <CenterLongitude><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/CIRCLE/CENTERLONGITUDE/VALUE"/></CenterLongitude>
                                            <Radius><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/CIRCLE/RADIUSVALUE/VALUE"/></Radius>
                                    <xsl:if test="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/CIRCLE/RADIUSUNITS">
                                            <RadiusUnits><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/CIRCLE/RADIUSUNITS/VALUE"/></RadiusUnits>
                                    </xsl:if>
                                        </Circle>	
                                    </xsl:if>
                                    
                                    <xsl:if test="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/BOUNDINGRECTANGLE">
                                        <BoundingRectangle>				
                                            <WestBoundingCoordinate><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/BOUNDINGRECTANGLE/WESTBOUNDINGCOORDINATE/VALUE"/></WestBoundingCoordinate>
                                            <NorthBoundingCoordinate><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/BOUNDINGRECTANGLE/NORTHBOUNDINGCOORDINATE/VALUE"/></NorthBoundingCoordinate>
                                            <EastBoundingCoordinate><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/BOUNDINGRECTANGLE/EASTBOUNDINGCOORDINATE/VALUE"/></EastBoundingCoordinate>
                                            <SouthBoundingCoordinate><xsl:value-of select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/BOUNDINGRECTANGLE/SOUTHBOUNDINGCOORDINATE/VALUE"/></SouthBoundingCoordinate>
                                        </BoundingRectangle>
                                    </xsl:if>
                                    
                                    <xsl:if test="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/GPOLYGON/GPOLYGONCONTAINER">
                                        <GPolygon>
                                            <xsl:for-each select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/GPOLYGON/GPOLYGONCONTAINER">
                                                <Boundary>	                                    
                                                    <xsl:for-each select="./GRINGPOINT/GRINGPOINTLATLON/POINT">
                                                        <Point>                                                           
                                                            <PointLongitude>
                                                                <xsl:value-of  select="./GRINGPOINTLONGITUDE"/>                                                                
                                                            </PointLongitude>
                                                            <PointLatitude>
                                                                <xsl:value-of  select="./GRINGPOINTLATITUDE"/>                                                                
                                                            </PointLatitude>
                                                        </Point>
                                                    </xsl:for-each>
                                                </Boundary>
                                            </xsl:for-each>
                                        </GPolygon>
                                    </xsl:if>			
                                    
                                    <xsl:if test="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/POINT">
                                        <Point>	                                            
                                            <PointLongitude>
                                                <xsl:value-of  select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/POINT/POINTLONGITUDE/VALUE"/>
                                            </PointLongitude>
                                            <PointLatitude>
                                                <xsl:value-of  select="./SPATIALDOMAINCONTAINER/HORIZONTALSPATIALDOMAINCONTAINER/POINT/POINTLATITUDE/VALUE"/>
                                            </PointLatitude>
                                        </Point> 
                                    </xsl:if>                                   
                                </HorizontalSpatialDomainContainer>	 
                            </xsl:if>
                        </SpatialDomainContainer>
                    </xsl:if>     <!-- end if test="$foundValue != '' "-->
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL OrbitCalculatedSpatialDomain (OrbitCalculatedSpatialDomainContainer)+  -->		                                         
                <!-- ****************************************************************************** -->
                <xsl:if test="child::ORBITCALCULATEDSPATIALDOMAIN">
                    <OrbitCalculatedSpatialDomain>
                        <xsl:for-each select="./ORBITCALCULATEDSPATIALDOMAIN/ORBITCALCULATEDSPATIALDOMAINCONTAINER">
                            <!-- ****************************************************************************** -->
                            <!-- OrbitCalculatedSpatialDomain/OrbitCalculatedSpatialDomainContainer             -->
                            <!-- (OrbitalModelName?, ((OrbitNumber) | (StartOrbitNumber? , StopOrbitNumber?)),  -->
                            <!-- EquatorCrossingLongitude, EquatorCrossingDate, EquatorCrossingTime)            -->	                                         
                            <!-- ****************************************************************************** -->
                            <OrbitCalculatedSpatialDomainContainer>
                                <xsl:if test="./ORBITALMODELNAME/VALUE != ''">
                                    <xsl:for-each select="./ORBITALMODELNAME">
                                        <OrbitalModelName><xsl:value-of select="./VALUE"/></OrbitalModelName>
                                    </xsl:for-each>
                                </xsl:if>			
                                <xsl:if test="./ORBITNUMBER/VALUE != ''">
                                    <xsl:for-each select="./ORBITNUMBER">
                                        <OrbitNumber><xsl:value-of select="./VALUE"/></OrbitNumber>
                                    </xsl:for-each>
                                </xsl:if>			
                <xsl:if test="child::STARTORBITNUMBER">
<OrbitRange>
                                <xsl:if test="./STARTORBITNUMBER/VALUE != ''">
                                    <xsl:for-each select="./STARTORBITNUMBER">
                                          <StartOrbitNumber><xsl:value-of select="./VALUE"/></StartOrbitNumber>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./STOPORBITNUMBER/VALUE != ''">
                                    <xsl:for-each select="./STOPORBITNUMBER">
                                          <StopOrbitNumber><xsl:value-of select="./VALUE"/></StopOrbitNumber>
                                    </xsl:for-each>
                                </xsl:if>
                                        </OrbitRange>
                                </xsl:if>			
                                <xsl:if test="./EQUATORCROSSINGLONGITUDE/VALUE != ''">
                                    <xsl:for-each select="./EQUATORCROSSINGLONGITUDE">
                                        <EquatorCrossingLongitude><xsl:value-of select="./VALUE"/></EquatorCrossingLongitude>
                                    </xsl:for-each>
                                </xsl:if>			
                                <xsl:if test="./EQUATORCROSSINGDATE/VALUE != ''">
                                    <xsl:for-each select="./EQUATORCROSSINGDATE">
                                        <EquatorCrossingDate><xsl:value-of select="./VALUE"/></EquatorCrossingDate>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./EQUATORCROSSINGTIME/VALUE != ''">
                                    <xsl:for-each select="./EQUATORCROSSINGTIME">
                                        <EquatorCrossingTime><xsl:value-of select="./VALUE"/></EquatorCrossingTime>
                                    </xsl:for-each>
                                </xsl:if>      
                            </OrbitCalculatedSpatialDomainContainer>
                        </xsl:for-each>
                    </OrbitCalculatedSpatialDomain>
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL  MeasuredParameter  (MeasuredParameterContainer)+                      -->	                                           
                <!-- ****************************************************************************** -->
	
                <!-- Workaround for bug in SDSRV in which it generates an empty QAStats -->
                <!-- if no VALUES are found in QAStats,  it's an empty one and -->
                <!--     must be ignored.  -->
                <xsl:variable name="foundQAStatsValue">          
                    <xsl:for-each select="./MEASUREDPARAMETER/MEASUREDPARAMETERCONTAINER/QASTATS//VALUE">
                        <xsl:choose>              
                            <xsl:when test=". != '' ">      
                                <xsl:text>1</xsl:text>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:text></xsl:text>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:for-each>
                </xsl:variable> 
                
                <xsl:variable name="foundQAFlagsValue">          
                    <xsl:for-each select="./MEASUREDPARAMETER/MEASUREDPARAMETERCONTAINER/QAFLAGS//VALUE">
                        <xsl:choose>              
                            <xsl:when test=". != '' ">      
                                <xsl:text>1</xsl:text>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:text></xsl:text>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:for-each>
                </xsl:variable>          
                
                <xsl:if test="child::MEASUREDPARAMETER"> 
                    <MeasuredParameter>    		
                        
                        <xsl:for-each select="./MEASUREDPARAMETER/MEASUREDPARAMETERCONTAINER">
                            <MeasuredParameterContainer>
                                <!-- Might have one or more MeasuredParameterContainers, each containing -->
                                <!-- one ParameterName and the corresponding QAStats and QAFlags         -->
		
                                <!-- ****************************************************************************** -->
                                <!-- MeasuredParameter/MeasuredParameterContainer                                   -->
                                <!-- (ParameterName, QAStats?, QAFlags?)                                            -->	                                           
                                <!-- ****************************************************************************** -->
                                <xsl:if test="./PARAMETERNAME/VALUE != ''">
                                    <ParameterName><xsl:value-of select="./PARAMETERNAME/VALUE"/></ParameterName>
                                </xsl:if>
                                <xsl:if test="./QASTATS">
                                    <xsl:for-each select="./QASTATS">
                                        <!-- ****************************************************************************** -->
                                        <!-- QAStats                                                                        -->
                                        <!-- (QAPercentMissingData, QAPercentOutofBoundsData?, QAPercentInterpolatedData?,  -->
                                        <!-- QAPercentCloudCover?)                                                          -->	                                           
                                        <!-- ****************************************************************************** -->
                                        <xsl:if test="$foundQAStatsValue != '' ">
                                            <QAStats>				
                                                <xsl:if test="./QAPERCENTMISSINGDATA/VALUE != ''">
                                                    <QAPercentMissingData><xsl:value-of select="./QAPERCENTMISSINGDATA/VALUE "/></QAPercentMissingData>
                                                </xsl:if>
                                                <xsl:if test="./QAPERCENTOUTOFBOUNDSDATA/VALUE != ''">
                                                    <QAPercentOutofBoundsData><xsl:value-of select="./QAPERCENTOUTOFBOUNDSDATA/VALUE"/></QAPercentOutofBoundsData>
                                                </xsl:if>
                                                <xsl:if test="./QAPERCENTINTERPOLATEDDATA/VALUE != ''">
                                                    <QAPercentInterpolatedData><xsl:value-of select="./QAPERCENTINTERPOLATEDDATA/VALUE"/></QAPercentInterpolatedData>
                                                </xsl:if> 
                                                <xsl:if test="./QAPERCENTCLOUDCOVER/VALUE != ''">
                                                    <QAPercentCloudCover><xsl:value-of select="./QAPERCENTCLOUDCOVER/VALUE"/></QAPercentCloudCover>
                                                </xsl:if>			
                                            </QAStats>
                                        </xsl:if>	
                                    </xsl:for-each>
                                </xsl:if>	
                                <xsl:if test="./QAFLAGS">
                                    <xsl:for-each select="./QAFLAGS">
                                        <!-- ****************************************************************************** -->
                                        <!-- QAFlags                                                                        -->
                                        <!-- (AutomaticQualityFlag, AutomaticQualityFlagExplanation,                        -->
                                        <!-- OperationalQualityFlag?, OperationalQualityFlagExplanation?,                   -->
                                        <!-- ScienceQualityFlag?, ScienceQualityFlagExplanation?)                           -->
                                        <!-- QAPercentCloudCover?)                                                          -->	                                           
                                        <!-- ****************************************************************************** -->
                                        <xsl:if test="$foundQAFlagsValue != '' ">			
                                            <QAFlags>
                                                <!-- handle the bugs in m2xt DTD. Put DummyValue in order to pass the validation -->
                                                <xsl:choose>
                                                    <xsl:when test="./AUTOMATICQUALITYFLAG/VALUE != ''">
                                                        <AutomaticQualityFlag><xsl:value-of select="./AUTOMATICQUALITYFLAG/VALUE"/></AutomaticQualityFlag> 
                                                    </xsl:when>
                                                    <xsl:otherwise>
                                                        <AutomaticQualityFlag>DummyValue</AutomaticQualityFlag> 
                                                    </xsl:otherwise>	
                                                </xsl:choose>
                                                <!-- handle the bugs in m2xt DTD. Put DummyValue in order to pass the validation -->
                                                <xsl:choose>
                                                    <xsl:when test="./AUTOMATICQUALITYFLAGEXPLANATION/VALUE != ''">
                                                        <AutomaticQualityFlagExplanation><xsl:value-of select="./AUTOMATICQUALITYFLAGEXPLANATION/VALUE"/></AutomaticQualityFlagExplanation>
                                                    </xsl:when>
                                                    <xsl:otherwise>
                                                        <AutomaticQualityFlagExplanation>DummyValue</AutomaticQualityFlagExplanation> 
                                                    </xsl:otherwise>	
                                                </xsl:choose>
                                                <xsl:if test="./OPERATIONALQUALITYFLAG/VALUE != ''">
                                                    <OperationalQualityFlag><xsl:value-of select="./OPERATIONALQUALITYFLAG/VALUE"/></OperationalQualityFlag>
                                                </xsl:if>
                                                <xsl:if test="./OPERATIONALQUALITYFLAGEXPLANATION/VALUE != ''">
                                                    <OperationalQualityFlagExplanation><xsl:value-of select="./OPERATIONALQUALITYFLAGEXPLANATION/VALUE"/></OperationalQualityFlagExplanation>
                                                </xsl:if>
                                                <xsl:if test="./SCIENCEQUALITYFLAG/VALUE != ''">
                                                    <ScienceQualityFlag><xsl:value-of select="./SCIENCEQUALITYFLAG/VALUE"/></ScienceQualityFlag>
                                                </xsl:if>
                                                <xsl:if test="./SCIENCEQUALITYFLAGEXPLANATION/VALUE != ''">
                                                    <ScienceQualityFlagExplanation><xsl:value-of select="./SCIENCEQUALITYFLAGEXPLANATION/VALUE"/></ScienceQualityFlagExplanation>
                                                </xsl:if>
                                            </QAFlags>
                                        </xsl:if>	
                                    </xsl:for-each>
                                </xsl:if>				
                            </MeasuredParameterContainer>
                        </xsl:for-each>
                    </MeasuredParameter>
                </xsl:if>
                <!-- ****************************************************************************** -->
                <!-- The DPL  ProcessingQA (ProcessingQAContainer)+                                 -->
                <!-- ProcessingQAContainer (ProcessingQADescription?, ProcessingQAAttribute?)       -->                                         
                <!-- ****************************************************************************** -->
	
                <xsl:if test="child::PROCESSINGQA">
                    <ProcessingQA>
                        <xsl:for-each select="./PROCESSINGQA/PROCESSINGQACONTAINER">
                            <ProcessingQAContainer>
                                <xsl:if test="./PROCESSINGQADESCRIPTION/VALUE != ''">
                                    <xsl:for-each select="./PROCESSINGQADESCRIPTION">
                                        <ProcessingQADescription><xsl:value-of select="./VALUE"/></ProcessingQADescription>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./PROCESSINGQAATTRIBUTE/VALUE != ''">
                                    <xsl:for-each select="./PROCESSINGQAATTRIBUTE">
                                        <ProcessingQAAttribute><xsl:value-of select="./VALUE"/></ProcessingQAAttribute>
                                    </xsl:for-each>	
                                </xsl:if>					
                            </ProcessingQAContainer>
                        </xsl:for-each>
                    </ProcessingQA>
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL  StorageMediumClass (StorageMedium)+                                   -->	                                 
                <!-- ****************************************************************************** -->
                <xsl:if test="child::STORAGEMEDIUMCLASS">
                    <StorageMediumClass>
                        <xsl:for-each select="./STORAGEMEDIUMCLASS/STORAGEMEDIUMCLASS">
                            <StorageMedium><xsl:value-of select="./STORAGEMEDIUMCLASS/STORAGEMEDIUM/VALUE"/></StorageMedium>
                        </xsl:for-each>
                    </StorageMediumClass>
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL  Review (ReviewContainer)+                                             -->	
                <!-- ReviewContainer (ScienceReviewStatus?, ScienceReviewDate?, FutureReviewDate?)  -->                               
                <!-- ****************************************************************************** -->	
                <xsl:if test="child::REVIEW">
                    <Review>
                        <xsl:for-each select="./REVIEW/REVIEWCONTAINER">
                            <ReviewContainer>
                                <xsl:if test="./SCIENCEREVIEWSTATUS/VALUE != ''">
                                    <xsl:for-each select="./SCIENCEREVIEWSTATUS">
                                        <ScienceReviewStatus><xsl:value-of select="./VALUE"/></ScienceReviewStatus>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./SCIENCEREVIEWDATE/VALUE != ''">
                                    <xsl:for-each select="./SCIENCEREVIEWDATE">
                                        <ScienceReviewDate><xsl:value-of select="./VALUE"/></ScienceReviewDate>
                                    </xsl:for-each>
                                </xsl:if>			
                                <xsl:if test="./FUTUREREVIEWDATE/VALUE != ''">
                                    <xsl:for-each select="./FUTUREREVIEWDATE">
                                        <FutureReviewDate><xsl:value-of select="./VALUE"/></FutureReviewDate>
                                    </xsl:for-each>
                                </xsl:if>
                            </ReviewContainer>
                        </xsl:for-each>
                    </Review>
                </xsl:if>	
                
                <!-- ****************************************************************************** -->
                <!-- The DPL Platform (PlatformShortName,Instrument*)                               -->	
                <!-- Instrument (InstrumentShortName, Sensor*, OperationMode*)                      --> 
                <!-- Sensor (SensorShortName, SensorCharacteristic*)                                -->                                                     
                <!-- ****************************************************************************** -->
	

                <xsl:if test="child::SENSORCHARACTERISTIC">
                    <xsl:for-each select="./SENSORCHARACTERISTIC/SENSORCHARACTERISTICCONTAINER">		
                        <xsl:if test="./PLATFORMSHORTNAME/VALUE != ''">
                            <Platform>
                                <xsl:for-each select="./PLATFORMSHORTNAME">
                                    <PlatformShortName><xsl:value-of select="./VALUE"/></PlatformShortName>
                                </xsl:for-each>
                                
                                <xsl:if test="./INSTRUMENTSHORTNAME/VALUE != ''">
                                    <Instrument>
                                        <xsl:for-each select="./INSTRUMENTSHORTNAME">
                                            <InstrumentShortName><xsl:value-of select="./VALUE"/></InstrumentShortName>
                                        </xsl:for-each>	
                                        
                                        <xsl:if test="./SENSORSHORTNAME/VALUE != ''">
                                            <Sensor>
                                                <xsl:for-each select="./SENSORSHORTNAME">
                                                    <SensorShortName><xsl:value-of select="./VALUE"/></SensorShortName>
                                                </xsl:for-each>
                                                <xsl:if test="//SENSORCHARACTERISTICNAME/VALUE != ''">
                                                    <xsl:variable name="sensorName">	
                                                        <xsl:value-of select="//SENSORCHARACTERISTICNAME/VALUE"/>
                                                    </xsl:variable>
                                                    
                                                    <xsl:variable name="sensorValue">	
                                                        <xsl:value-of select="//SENSORCHARACTERISTICVALUE/VALUE"/>
                                                    </xsl:variable>
                                                    
                                                    <SensorCharacteristic>
                                                        
                                                        <SensorCharacteristicName><xsl:value-of select="$sensorName"/></SensorCharacteristicName>
                                                        
                                                        
                                                        <SensorCharacteristicValue><xsl:value-of select="$sensorValue"/></SensorCharacteristicValue>
                                                        
                                                    </SensorCharacteristic>
                                                </xsl:if>
                                            </Sensor>	
                                        </xsl:if>
                                    </Instrument>		
                                </xsl:if>	
                            </Platform>
                        </xsl:if>	
                    </xsl:for-each>	
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL Platform (PlatformShortName,Instrument*)                               -->	
                <!-- Instrument (InstrumentShortName, Sensor*, OperationMode*)                      --> 
                <!-- Sensor (SensorShortName, SensorCharacteristic*)                                -->                                                     
                <!-- The SensorCharacteristic is not handled according to the current DTD. But since it is not used by any science team, we opt to fix it when necessary. The fix would be more than just xsl changes, it will require DTD changes as well.
                    -->                                                     
                <!-- ****************************************************************************** -->	
                <xsl:copy-of select="//Platform" />

                <!-- ****************************************************************************** -->
                <!-- The DPL AnalysisSource   (AnalysisShortName)                                   -->	
                <!-- The input metadata won't has "AnalysisSource" attribute. However, it is        -->
                <!-- a required element in the m2xt DTD. In order to pass the DTD validation,       -->
                <!-- a "dummyValue" has been added in the XML file as temporary solution.           -->	                                                   
                <!-- ****************************************************************************** -->
                <xsl:if test="child::ANALYSISSOURCE">
                    <AnalysisSource>		
                        <xsl:if test="./ANALYSISSOURCE/ANALYSISSHORTNAME/VALUE != ''">
                            <xsl:for-each select="./ANALYSISSOURCE/ANALYSISSHORTNAME">
                                <AnalysisShortName><xsl:value-of select="./VALUE"/></AnalysisShortName>
                            </xsl:for-each>
                        </xsl:if>		
                    </AnalysisSource>
                </xsl:if>
                <!-- ****************************************************************************** -->
                <!-- The DPL Campaign (CampaignShortName)                                           -->	                                                 
                <!-- ****************************************************************************** -->	
	
                <xsl:if test="child::CAMPAIGN">
                    <Campaign>
                        <xsl:for-each select="./CAMPAIGN">
                            <CampaignShortName><xsl:value-of select="./CAMPAIGNSHORTNAME/VALUE"/></CampaignShortName>
                        </xsl:for-each>
                    </Campaign>
                </xsl:if>
                
                <!-- ****************************************************************************** -->
                <!-- The DPL PSAs (PSA+)                                                            -->
                <!-- PSA (PSAName, PSAValue*)                                                       -->
                <!-- <PSAValue>value1</PSAValue> by applying "create_parametervalue" template       -->	                                                 
                <!-- ****************************************************************************** -->	
	
                <xsl:if test="child::ADDITIONALATTRIBUTES">
                    <PSAs>
                        <xsl:for-each select="./ADDITIONALATTRIBUTES/ADDITIONALATTRIBUTESCONTAINER">
                            <PSA>
                                <PSAName><xsl:value-of select="./ADDITIONALATTRIBUTENAME/VALUE"/></PSAName>
                                <xsl:for-each select="./INFORMATIONCONTENT/PARAMETERVALUE/VALUE">
                                    <PSAValue><xsl:value-of select = "."/></PSAValue>
                                </xsl:for-each>			
                            </PSA>
                        </xsl:for-each>
                    </PSAs>
                </xsl:if>
                
                
                
                <!-- ****************************************************************************** -->
                <!-- The DPL InputGranule (InputPointer+)                                           -->	 
                <!-- <InputPointer>value1</InputPointer>                                            --> 
                <!-- <InputPointer>value2</InputPointer>                                            --> 
                <!-- <InputPointer>value3</InputPointer>                                            -->                                             
                <!-- ****************************************************************************** -->
	
                <xsl:if test="child::INPUTGRANULE">
                    <InputGranule>
                        <xsl:for-each select="./INPUTGRANULE/INPUTPOINTER">	
                            <xsl:for-each select="./VALUE">
                                <InputPointer><xsl:value-of select = "."/></InputPointer>	
                            </xsl:for-each> 
                        </xsl:for-each>	
                    </InputGranule>
                </xsl:if>
                
                
                
                <xsl:if test="child::ANCILLARYINPUTGRANULE">
                    <AncillaryInputGranules>
                        <xsl:for-each select="./ANCILLARYINPUTGRANULE/ANCILLARYINPUTGRANULECONTAINER">
                            <AncillaryInputGranule>
                                <xsl:if test="./ANCILLARYINPUTTYPE/VALUE != ''">
                                    <xsl:for-each select="./ANCILLARYINPUTTYPE">
                                        <AncillaryInputType><xsl:value-of select="./VALUE"/></AncillaryInputType>
                                    </xsl:for-each>
                                </xsl:if>
                                <xsl:if test="./ANCILLARYINPUTPOINTER/VALUE != ''">
                                    <xsl:for-each select="./ANCILLARYINPUTPOINTER">
                                        <AncillaryInputPointer><xsl:value-of select="./VALUE"/></AncillaryInputPointer>
                                    </xsl:for-each>
                                </xsl:if>
                            </AncillaryInputGranule>
                        </xsl:for-each>
                    </AncillaryInputGranules>
                </xsl:if>
                
            </GranuleURMetaData>
        </GranuleMetaDataFile> 			
    </xsl:template>
    
    <!-- ****************************************************************************** -->
    <!-- reformat template                                                              -->
    <!-- This template is used to reformat the datetime                                 -->
    <!-- to be the DPL XML acceptable format by removing "T" and "Z" characters         --> 
    <!-- ****************************************************************************** -->
    <xsl:template name="reformat">
        <xsl:param name="dateTime"/>
        <xsl:choose>
            <xsl:when test="contains($dateTime, 'T')">
                <xsl:call-template name="reformat">
                    <xsl:with-param name="dateTime">
                        <xsl:value-of select ="concat(substring-before($dateTime,'T'),' ',substring-after($dateTime, 'T'))"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="contains($dateTime, 'Z')">
                <xsl:call-template name="reformat">
                    <xsl:with-param name="dateTime">	
                        <xsl:value-of select ="substring-before($dateTime,'Z')"/>
                    </xsl:with-param>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="contains($dateTime, '.')">
                        <xsl:value-of select ="concat(substring-before($dateTime,'.'),'.',substring(substring-after($dateTime, '.'), 1, 3))"/>
            </xsl:when>
            <xsl:otherwise>	
                <xsl:value-of select ="$dateTime"/>	
            </xsl:otherwise>
        </xsl:choose>	
    </xsl:template>	
    
    <!-- ****************************************************************************** -->
    <!-- reformat_rangetime template                                                    -->
    <!-- This template is used to reformat the rangetime                                -->
    <!-- to be the DPL XML acceptable format by removing the last char if it is 'Z' and padding to nanosecond precision (i.e. .123456) if neccesary                --> 
    <!-- ****************************************************************************** -->
    <xsl:template name="reformat_rangetime">
      <xsl:param name="time"/>
      <xsl:choose>
        <xsl:when test="substring($time, string-length($time)) = 'Z'">
          <xsl:call-template name="reformat_rangetime">
            <xsl:with-param name="time">
              <xsl:value-of select ="substring($time, 1, string-length($time)-1)"/>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="contains($time, '.')">
              <xsl:value-of select ="concat(substring-before($time,'.'), '.', substring(concat(substring-after($time, '.'), '000000'), 1, 6))"/>
            </xsl:when>
            <xsl:otherwise>	
              <xsl:value-of select ="concat($time,'.000000')"/>
            </xsl:otherwise>
          </xsl:choose>	
        </xsl:otherwise>
      </xsl:choose>
    </xsl:template>	
    <!-- ****************************************************************************** -->
    <!-- reformat_rangedate template                                                    -->
    <!-- This template is used to reformat the RangeDate                                -->
    <!-- to be the DPL XML acceptable format by removing the T at the end if neccesary                --> 
    <!-- ****************************************************************************** -->
    <xsl:template name="reformat_rangedate">
        <xsl:param name="date"/>
        <xsl:choose>
            <xsl:when test="contains($date, 'T')">
                <xsl:value-of select ="substring-before($date, 'T')"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select ="$date"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
